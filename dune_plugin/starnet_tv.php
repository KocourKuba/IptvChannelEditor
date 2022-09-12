<?php
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_channel.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/hashed_array.php';
require_once 'lib/epg_xml_parser.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';


class Starnet_Tv extends Abstract_Tv
{
    /**
     * @var Starnet_Plugin
     */
    protected $plugin;

    /**
     * @param Starnet_Plugin $plugin
     */
    public function __construct(Starnet_Plugin $plugin)
    {
        $this->plugin = $plugin;
        parent::__construct(Abstract_Tv::MODE_CHANNELS_N_TO_M, false);
    }

    /**
     * @return string
     */
    public function get_fav_icon_url()
    {
        return Default_Config::FAV_CHANNEL_GROUP_ICON_PATH;
    }

    /**
     * @return bool
     */
    public function is_favorites_supported()
    {
        return $this->plugin->config->get_feature(TV_FAVORITES_SUPPORTED);
    }

    /**
     * @param array &$items
     */
    public function add_special_groups(&$items)
    {
        if ($this->plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url =>
                    MediaURL::encode(
                        array
                        (
                            'screen_id' => Starnet_Vod_Category_List_Screen::ID,
                            'name' => 'VOD',
                        )),
                PluginRegularFolderItem::caption => Default_Config::VOD_GROUP_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Config::VOD_GROUP_ICON
                )
            );
        }
    }

    /**
     * @param $plugin_cookies
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $channels_list_path = '';
        try {
            $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
            $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;
            switch ($source) {
                case 1:
                    $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path') . $channels_list;
                    break;
                case 2:
                    if (isset($plugin_cookies->channels_url) && !empty($plugin_cookies->channels_url)) {
                        $url_path = $plugin_cookies->channels_url;
                    } else {
                        $url_path = $this->plugin->config->PLUGIN_CHANNELS_URL_PATH;
                    }

                    $channels_list_path = get_temp_path($channels_list);
                    if (!file_exists($channels_list_path)) {
                        file_put_contents($channels_list_path, HD::http_get_document($url_path . $channels_list));
                    }
                    break;
            }

            $xml = HD::parse_xml_file($channels_list_path);
        } catch (Exception $ex) {
            hd_print("Can't fetch channel_list $channels_list_path");
            return;
        }

        if ($xml->getName() !== 'tv_info') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception('Invalid XML document');
        }

        // Create channels and groups
        $this->channels = new Hashed_Array();
        $this->groups = new Hashed_Array();

        // Favorites group
        if ($this->is_favorites_supported()) {
            $this->groups->put(new Favorites_Group($this,
                Default_Config::FAV_CHANNEL_GROUP_ID,
                Default_Config::FAV_CHANNEL_GROUP_CAPTION,
                Default_Config::FAV_CHANNEL_GROUP_ICON_PATH));
        }

        // All channels group
        $this->groups->put(new All_Channels_Group($this,
            Default_Config::ALL_CHANNEL_GROUP_CAPTION,
            Default_Config::ALL_CHANNEL_GROUP_ICON_PATH));

        // read category
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = "Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            if (isset($xml_tv_category->disabled)) {
                continue;
            }

            $this->groups->put(new Default_Group((string)$xml_tv_category->id,
                (string)$xml_tv_category->caption,
                (string)$xml_tv_category->icon_url));
        }

        $this->plugin->config->GetAccountInfo($plugin_cookies);
        $pl_entries = $this->plugin->config->GetPlaylistStreamInfo($plugin_cookies);

        // Read channels
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '{$xml_tv_channel->getName()}'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) {
                continue;
            }

            // update play stream url and calculate unique id from url hash
            if (isset($xml_tv_channel->channel_id)) {
                $channel_id = (string)$xml_tv_channel->channel_id;
                $ext_params = isset($pl_entries[$channel_id]) ? $pl_entries[$channel_id] : array();
                // update stream url by channel ID
                $url = $this->plugin->config->UpdateStreamUrlID($channel_id, $ext_params);
                if (empty($url)) {
                    continue;
                }
                $streaming_url = '';
                $hash = hash("crc32", $url);
            } else {
                // custom url, play as is
                $streaming_url = (string)$xml_tv_channel->streaming_url;
                $hash = $channel_id = hash("crc32", $streaming_url);
                $ext_params = array();
            }
            // hd_print("load_channels: $streaming_url");

            // Read category id from channel
            if (!isset($xml_tv_channel->tv_category_id)) {
                hd_print("Error: Category undefined for channel $hash ($xml_tv_channel->caption) !");
                continue;
            }

            $tv_category_id = (int)$xml_tv_channel->tv_category_id;
            if (!$this->groups->has($tv_category_id)) {
                // Category disabled or some went wrong
                continue;
            }

            if ($this->channels->has($hash)) {
                $channel = $this->channels->get($hash);
            } else {
                // https not supported for old players
                // $icon_url = str_replace("https://", "http://", (string)$xml_tv_channel->icon_url);
                $icon_url = (string)$xml_tv_channel->icon_url;
                $number = isset($xml_tv_channel->int_id) ? (int)$xml_tv_channel->int_id : 0;

                $epg1 = (string)$xml_tv_channel->epg_id;
                $epg2 = (empty($xml_tv_channel->tvg_id)) ? $epg1 : (string)$xml_tv_channel->tvg_id;

                $channel = new Default_Channel(
                    $hash,
                    $channel_id,
                    (string)$xml_tv_channel->caption,
                    $icon_url,
                    $streaming_url,
                    (int)$xml_tv_channel->archive,
                    $number,
                    $epg1,
                    $epg2,
                    (int)$xml_tv_channel->protected,
                    (int)$xml_tv_channel->timeshift_hours,
                    $ext_params
                );
                $this->channels->put($channel);
            }

            // Link group and channel.
            $group = $this->groups->get($tv_category_id);
            $channel->add_group($group);
            $group->add_channel($channel);

            // only new format support favorites
            if ($xml_tv_channel->favorite && $this->is_favorites_supported()) {
                hd_print("Add to favorite $hash ($xml_tv_channel->caption)");
                $this->change_tv_favorites(PLUGIN_FAVORITES_OP_ADD, $hash, $plugin_cookies);
            }
        }

        hd_print("Loaded: channels: {$this->channels->size()}, groups: {$this->groups->size()}");
    }

    /**
     * @param string $playback_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        return $playback_url;
    }

    /**
     * @param string $channel_id
     * @param int $archive_ts
     * @param string $protect_code
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        try {
            $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
            // get channel by hash
            $channel = $this->get_channel($channel_id);
            if ($protect_code !== $pass_sex && $channel->is_protected()) {
                hd_print('Wrong adult password');
                throw new Exception('Wrong adult password');
            }
        } catch (Exception $ex) {
            hd_print("get_tv_playback_url: Exception " . $ex->getMessage());
            return '';
        }

        // update url if play archive or different type of the stream
        $url = $this->plugin->config->TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        hd_print("get_tv_playback_url: $url");
        return $url;
    }

    /**
     * @param string $channel_id
     * @param int $day_start_ts
     * @param $plugin_cookies
     * @return array|Epg_Iterator
     */
    public function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies)
    {
        try {
            // get channel by hash
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;

        $epg_man = new Epg_Manager($this->plugin->config);

        try {
            $day_start_ts -= get_local_time_zone_offset();
            $epg = $epg_man->get_epg($channel, $epg_source, $day_start_ts, $plugin_cookies);
            if (count($epg) === 0) {
                hd_print("No data from $epg_source EPG");
                return array();
            }
        } catch (Exception $ex) {
            hd_print("Can't fetch EPG ID from $epg_source epg source: " . $ex->getMessage());
            return array();
        }

        hd_print("Loaded " . count($epg) . " EPG entries");
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;
        $epg_result = array();
        foreach ($epg as $time => $value) {
            $time_start = $time + $time_shift;
            $epg_result[] = new Default_Epg_Item($value['epg_title'], $value['epg_desc'], (int)$time_start, (int)$value['epg_end'] + $time_shift);
        }

        return new Epg_Iterator($epg_result, $day_start_ts, $day_start_ts + 86400);
    }
}

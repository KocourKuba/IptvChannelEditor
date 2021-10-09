<?php
require_once 'lib/hashed_array.php';
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/epg_xml_parser.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channel.php';
require_once 'starnet_vod_category_list_screen.php';


class StarnetPluginTv extends AbstractTv
{
    public static $config = null;

    public function __construct()
    {
        parent::__construct(AbstractTv::MODE_CHANNELS_N_TO_M, false);
    }

    public function get_fav_icon_url()
    {
        return DefaultConfig::FAV_CHANNEL_GROUP_ICON_PATH;
    }

    public function set_setup_screen($setup_screen)
    {
        $this->SettingsScreen = $setup_screen;
    }

    public function get_setup_screen()
    {
        return isset($this->SettingsScreen) ? $this->SettingsScreen : false;
    }

    public function is_favorites_supported()
    {
        $config = self::$config;
        return $config::$TV_FAVORITES_SUPPORTED;
    }

    public function get_channel_list_url($plugin_cookies)
    {
        $config = self::$config;
        return isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : $config::$CHANNELS_LIST;
    }

    public function add_special_groups(&$items)
    {
        $config = self::$config;
        if ($config::$VOD_MOVIE_PAGE_SUPPORTED) {

            array_unshift($items,
                array
                (
                    PluginRegularFolderItem::media_url =>
                        MediaURL::encode(
                            array
                            (
                                'screen_id' => StarnetVodCategoryListScreen::ID,
                                'name' => 'VOD',
                            )),
                    PluginRegularFolderItem::caption => DefaultConfig::VOD_GROUP_CAPTION,
                    PluginRegularFolderItem::view_item_params => array
                    (
                        ViewItemParams::icon_path => DefaultConfig::VOD_GROUP_ICON
                    )
                )
            );
        }
    }

    /**
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $config = self::$config;
        $channels_list = $this->get_channel_list_url($plugin_cookies);
        hd_print("Channels list: $channels_list");

        try {
            $doc = file_get_contents($channels_list, true);
            if($doc == false)
            {
                hd_print("File not exist! $channels_list");
                throw new Exception('File not exist');
            }
        } catch (Exception $ex) {
            hd_print("Can't fetch channel_list, alternative copy used.");
            return;
        }

        $xml = HD::parse_xml_document($doc);

        if ($xml->getName() !== 'tv_info') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception('Invalid XML document');
        }

        // read embedded access info
        $plugin_cookies->ott_key_local = "";
        $plugin_cookies->subdomain_local = "";
        $plugin_cookies->login_local = "";
        $plugin_cookies->password_local = "";

        if (isset($xml->channels_setup))
        {
            hd_print("Overriding access settings found in playlist: $channels_list");
            if ($config::$ACCOUNT_TYPE == 'OTT_KEY') {
                if (isset($xml->channels_setup->access_key)) {
                    $plugin_cookies->ott_key_local = strval($xml->channels_setup->access_key);
                    hd_print("access_key: $plugin_cookies->ott_key_local");
                }
                if (isset($xml->channels_setup->access_domain)) {
                    $plugin_cookies->subdomain_local = strval($xml->channels_setup->access_domain);
                    hd_print("subdomain: $plugin_cookies->subdomain_local");
                }
            }

            if ($config::$ACCOUNT_TYPE == 'LOGIN') {
                if (isset($xml->channels_setup->access_login)) {
                    $plugin_cookies->login_local = strval($xml->channels_setup->access_login);
                    hd_print("login: $plugin_cookies->login_local");
                }
            }

            if ($config::$ACCOUNT_TYPE == 'LOGIN' || $config::$ACCOUNT_TYPE == 'PIN') {
                if (isset($xml->channels_setup->access_password)) {
                    $plugin_cookies->password_local = strval($xml->channels_setup->access_password);
                    hd_print("password: $plugin_cookies->password_local");
                }
            }
        }

        $account_data = null;
        $config::GetAccountInfo($plugin_cookies, $account_data);
        $pl_entries = $config::GetPlaylistStreamInfo($plugin_cookies);

        // Create channels and groups
        $this->channels = new HashedArray();
        $this->groups = new HashedArray();

        // Favorites group
        if ($this->is_favorites_supported()) {
            $this->groups->put(new FavoritesGroup($this,
                DefaultConfig::FAV_CHANNEL_GROUP_ID,
                DefaultConfig::FAV_CHANNEL_GROUP_CAPTION,
                DefaultConfig::FAV_CHANNEL_GROUP_ICON_PATH));
        }

        // All channels group
        $this->groups->put(new AllChannelsGroup($this,
            DefaultConfig::ALL_CHANNEL_GROUP_CAPTION,
            DefaultConfig::ALL_CHANNEL_GROUP_ICON_PATH));

        // read category
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = "Error: unexpected node '" . $xml_tv_category->getName() . "'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            $this->groups->put(new DefaultGroup(strval($xml_tv_category->id),
                strval($xml_tv_category->caption),
                strval($xml_tv_category->icon_url)));
        }

        // Read channels
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '" . $xml_tv_channel->getName() . "'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) continue;

            // substitute template
            // calculate unique id from url hash
            if (isset($xml_tv_channel->channel_id)) {
                $channel_id = strval($xml_tv_channel->channel_id);
                if (isset($pl_entries[$channel_id]))
                {
                    $streaming_url = $pl_entries[$channel_id];
                    $hash = $channel_id = hash("crc32", $streaming_url);
                } else {
                    $streaming_url = $config::$MEDIA_URL_TEMPLATE_HLS;
                    $hash = hash("crc32", (str_replace('{ID}', $xml_tv_channel->channel_id, $config::$MEDIA_URL_TEMPLATE_HLS)));
                }
            } else {
                $streaming_url = strval($xml_tv_channel->streaming_url);
                $hash = $channel_id = hash("crc32", $streaming_url);
            }
           // hd_print("load_channels: $streaming_url");

            if ($this->channels->has($hash)) {
                // added or existing channel
                $channel = $this->channels->get($hash);
            }
            else
            {
                // https not supported for old players
                $icon_url = str_replace('https', 'https', strval($xml_tv_channel->icon_url));
                $number = isset($xml_tv_channel->int_id) ? intval($xml_tv_channel->int_id) : 0;
                $channel = new StarnetChannel(
                    $hash,
                    $channel_id,
                    strval($xml_tv_channel->caption),
                    $icon_url,
                    $streaming_url,
                    intval($xml_tv_channel->archive),
                    $number,
                    strval($xml_tv_channel->epg_id),
                    strval($xml_tv_channel->tvg_id),
                    intval($xml_tv_channel->protected),
                    intval($xml_tv_channel->timeshift_hours));

                $this->channels->put($channel);
            }

            // Read category id from channel
            if (isset($xml_tv_channel->tv_category_id)) {
                // new format
                $tv_category_id = intval($xml_tv_channel->tv_category_id);
                $group = $this->groups->get($tv_category_id);

                // Link group and channel.
                $channel->add_group($group);
                $group->add_channel($channel);

                // only new format support favorites
                if ($xml_tv_channel->favorite && $this->is_favorites_supported()) {
                    hd_print("Add to favorite $hash ($xml_tv_channel->caption)");
                    $this->change_tv_favorites(PLUGIN_FAVORITES_OP_ADD, $hash, $plugin_cookies);
                }
            } else if (isset($xml_tv_channel->tv_categories)) {
                // old format
                foreach ($xml_tv_channel->tv_categories->children() as $xml_tv_cat_id) {
                    if ($xml_tv_cat_id->getName() !== 'tv_category_id') {
                        hd_print("Error: unexpected node '" . $xml_tv_cat_id->getName() . "'. Expected: 'tv_category_id'");
                        throw new Exception('Invalid XML document');
                    }

                    $tv_category_id = intval($xml_tv_cat_id);
                    $group = $this->groups->get($tv_category_id);

                    // Link group and channel.
                    $channel->add_group($group);
                    $group->add_channel($channel);
                }
            } else {
                hd_print("Error: Category undefined for channel $hash ($xml_tv_channel->caption) !");
            }
        }

        hd_print("Loaded: channels: " . $this->channels->size() .", groups: " . $this->groups->size());
    }

    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        return $playback_url;
    }

    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        try {
            $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
            // get channel by hash
            $channel = $this->get_channel($channel_id);
            if ($channel->is_protected() && $protect_code !== $pass_sex) {
                throw new Exception('Wrong password');
            }
        } catch (Exception $ex) {
            hd_print("get_tv_playback_url: Exception " . $ex->getMessage());
            return '';
        }

        $url = self::$config->AdjustStreamUri($plugin_cookies, $archive_ts, $channel);
        hd_print("get_tv_playback_url: $url");
        return $url;
    }

    public function get_day_epg_iterator($channel_id, $day_start_ts, &$plugin_cookies)
    {
        try {
            // get channel by hash
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        $start = 0;
        $end = 0;
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;

        $epg_date = gmdate(DATE_ATOM, $day_start_ts);
        // hd_print("start date: $epg_date");
        $epg_result = array();
        $epg = self::$config->GetEPG($channel, $day_start_ts);
        foreach ($epg as $time => $value) {
            $tm =  $time + $time_shift;
            $end = $tm;
            if ($start == 0)
                $start = $tm;

            $epg_result[] = new DefaultEpgItem($value['title'], $value['desc'], intval($tm), -1);
        }

        return new EpgIterator($epg_result, $start, $end);
    }
}

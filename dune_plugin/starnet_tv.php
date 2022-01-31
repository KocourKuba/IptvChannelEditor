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
    public static $config;

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
        return self::$config->get_tv_fav_support();
    }

    public function get_channel_list_url($plugin_cookies)
    {
        return isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : self::$config->get_channel_list();
    }

    public function add_special_groups(&$items)
    {
        if (self::$config->get_vod_support()) {
            $items[] = array
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
            );
        }
    }

    /**
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $channels_list = $this->get_channel_list_url($plugin_cookies);
        hd_print("Channels list: $channels_list");
        $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path') . '/';
        hd_print("Channels list path: $channels_list_path");
        $channels_list_path .= $channels_list;

        try {
            $xml = HD::parse_xml_file($channels_list_path);
        } catch (Exception $ex) {
            hd_print("Can't fetch channel_list $channels_list_path");
            return;
        }

        if ($xml->getName() !== 'tv_info') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception('Invalid XML document');
        }

        // clear saved embedded info
        if (isset($plugin_cookies->ott_key_local)) {
            $plugin_cookies->ott_key_local = '';
        }
        if (isset($plugin_cookies->subdomain_local)) {
            $plugin_cookies->subdomain_local = '';
        }
        if (isset($plugin_cookies->login_local)) {
            $plugin_cookies->login_local = '';
        }
        if (isset($plugin_cookies->password_local)) {
            $plugin_cookies->password_local = '';
        }
        if (isset($plugin_cookies->mediateka_local)) {
            $plugin_cookies->mediateka_local = '';
        }

        // read embedded access info
        if (isset($xml->channels_setup)) {
            hd_print("Overriding access settings found in playlist: $channels_list");

            if (isset($xml->channels_setup->access_key)) {
                $plugin_cookies->ott_key_local = (string)$xml->channels_setup->access_key;
                hd_print("access_key: $plugin_cookies->ott_key_local");
            }

            if (isset($xml->channels_setup->access_domain)) {
                $plugin_cookies->subdomain_local = (string)$xml->channels_setup->access_domain;
                hd_print("subdomain: $plugin_cookies->subdomain_local");
            }

            if (isset($xml->channels_setup->access_login)) {
                $plugin_cookies->login_local = (string)$xml->channels_setup->access_login;
                hd_print("login: $plugin_cookies->login_local");
            }

            if (isset($xml->channels_setup->access_password)) {
                $plugin_cookies->password_local = (string)$xml->channels_setup->access_password;
                hd_print("password: " . base64_encode($plugin_cookies->password_local));
            }
        }

        if (isset($xml->portal_setup, $xml->portal_setup->portal_key)) {
            $plugin_cookies->mediateka_local = (string)$xml->portal_setup->portal_key;
            hd_print("portal_key: $plugin_cookies->mediateka_local");
        }

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
                $error_string = "Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            if (isset($xml_tv_category->disabled)) {
                continue;
            }

            $this->groups->put(new DefaultGroup((string)$xml_tv_category->id,
                (string)$xml_tv_category->caption,
                (string)$xml_tv_category->icon_url));
        }

        $account_data = array();
        self::$config->GetAccountInfo($plugin_cookies, $account_data);
        $pl_entries = self::$config->GetPlaylistStreamInfo($plugin_cookies);

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
                $streaming_url = self::$config->UpdateStreamUrlID($channel_id, $ext_params);
                if (empty($streaming_url)) {
                    continue;
                }
                $hash = hash("crc32", $streaming_url);
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

                $channel = new StarnetChannel(
                    $hash,
                    $channel_id,
                    (string)$xml_tv_channel->caption,
                    $icon_url,
                    $streaming_url,
                    (int)$xml_tv_channel->archive,
                    $number,
                    (string)$xml_tv_channel->epg_id,
                    (string)$xml_tv_channel->tvg_id,
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
            if ($protect_code !== $pass_sex && $channel->is_protected()) {
                throw new Exception('Wrong password');
            }
        } catch (Exception $ex) {
            hd_print("get_tv_playback_url: Exception " . $ex->getMessage());
            return '';
        }

        // update url if play archive or different type of the stream
        $url = self::$config->TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
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

        $epg_man = new EpgManager(self::$config);
        $epg = array();
        try {
            $epg = $epg_man->get_epg($channel, 'first', $day_start_ts, $plugin_cookies);
            if (count($epg) === 0) {
                throw new Exception("Empty first epg");
            }
        } catch (Exception $ex) {
            try {
                $epg = $epg_man->get_epg($channel, 'second', $day_start_ts, $plugin_cookies);
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from secondary epg source: " . $ex->getMessage());
            }
        }

        hd_print("Loaded " . count($epg) . " EPG entries");

        $start = 0;
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;

        $epg_result = array();
        foreach ($epg as $time => $value) {
            $tm = $time + $time_shift;
            if ($start === 0) {
                $start = $tm;
            }

            $epg_result[] = new DefaultEpgItem($value['title'], $value['desc'], (int)$tm, $value['end']);
        }

        return new EpgIterator($epg_result, $day_start_ts, $day_start_ts + 86400);
    }
}

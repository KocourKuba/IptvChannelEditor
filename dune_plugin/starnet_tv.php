<?php
require_once 'lib/hashed_array.php';
require_once 'lib/tv/tv.php';
require_once 'lib/tv/default_channel.php';
require_once 'lib/tv/all_channels_group.php';
require_once 'lib/tv/favorites_group.php';
require_once 'lib/tv/history_group.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/vod/vod_group.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';


class Starnet_Tv implements Tv, User_Input_Handler
{
    const ID = 'tv';
    const PARAM_ZOOM = 'zoom_select';
    const CHANNELS_ZOOM = 'channels_zoom';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @var Starnet_Plugin
     */
    protected $plugin;

    /**
     * @var bool
     */
    protected $show_all_channels_group;

    /**
     * @var int
     */
    protected $playback_runtime;

    /**
     * @var string
     */
    protected $playback_url_is_stream_url;

    /**
     * @template Channel
     * @var Hashed_Array<Channel>
     */
    protected $channels;

    /**
     * @var array
     */
    protected $epg_ids;

    /**
     * @template Group
     * @var Hashed_Array<Group>
     */
    protected $groups;

    /**
     * @var Group
     */
    protected $vod_group;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Starnet_Plugin $plugin
     */
    public function __construct(Starnet_Plugin $plugin)
    {
        $this->plugin = $plugin;
        $this->show_all_channels_group = true;
        $this->playback_url_is_stream_url = false;

        User_Input_Handler_Registry::get_instance()->register_handler($this);
    }

    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @return bool
     */
    public function is_favorites_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::TV_FAVORITES_SUPPORTED);
    }

    public function unload_channels()
    {
        $this->channels = null;
        $this->groups = null;
    }

    /**
     * @return Hashed_Array<Channel>
     */
    public function get_channels()
    {
        return $this->channels;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @return Channel|mixed
     */
    public function get_channel($channel_id)
    {
        $channels = $this->get_channels();
        if ($channels === null) {
            hd_print(__METHOD__ . ": Channels no loaded");
            return null;
        }

        $channel = $this->channels->get($channel_id);

        if (is_null($channel)) {
            hd_print(__METHOD__ . ": Unknown channel: $channel_id");
        }

        return $channel;
    }

    /**
     * @param Channel $channel
     */
    public function set_channel(Channel $channel)
    {
        $this->channels->set($channel);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @template Group
     * @return  Hashed_Array<Group>
     */
    public function get_groups()
    {
        return $this->groups;
    }

    /**
     * @return  Group
     */
    public function get_vod_group()
    {
        return $this->vod_group;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $group_id
     * @return Group|mixed
     * @throws Exception
     */
    public function get_group($group_id)
    {
        $g = $this->groups->get($group_id);

        if (is_null($g)) {
            hd_print(__METHOD__ . ": Unknown group: $group_id");
            throw new Exception(__METHOD__ . ": Unknown group: $group_id");
        }

        return $g;
    }

    /**
     * @param string $channel_id
     * @param $plugin_cookies
     * @return bool
     */
    public function is_favorite_channel_id($channel_id, $plugin_cookies)
    {
        if (!$this->is_favorites_supported()) {
            return false;
        }

        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);
        return in_array($channel_id, $fav_channel_ids);
    }

    /**
     * @param string $fav_op_type
     * @param string $channel_id
     * @param $plugin_cookies
     * @return array
     */
    public function change_tv_favorites($fav_op_type, $channel_id, &$plugin_cookies)
    {
        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        switch ($fav_op_type) {
            case PLUGIN_FAVORITES_OP_ADD:
                if (in_array($channel_id, $fav_channel_ids) === false) {
                    hd_print(__METHOD__ . ": Add channel $channel_id to favorites");
                    $fav_channel_ids[] = $channel_id;
                }
                break;
            case PLUGIN_FAVORITES_OP_REMOVE:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false) {
                    hd_print(__METHOD__ . ": Remove channel $channel_id from favorites");
                    unset ($fav_channel_ids[$k]);
                }
                break;
            case ACTION_CLEAR_FAVORITES:
                hd_print(__METHOD__ . ": Clear favorites");
                $fav_channel_ids = array();
                break;
            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== 0) {
                    hd_print(__METHOD__ . ": Move channel $channel_id up");
                    $t = $fav_channel_ids[$k - 1];
                    $fav_channel_ids[$k - 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== count($fav_channel_ids) - 1) {
                    hd_print(__METHOD__ . ": Move channel $channel_id down");
                    $t = $fav_channel_ids[$k + 1];
                    $fav_channel_ids[$k + 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

        $media_urls = array(Starnet_Tv_Favorites_Screen::get_media_url_str(),
            Starnet_Tv_Channel_List_Screen::get_media_url_str(Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID));

        Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
        $post_action = Starnet_Epfs_Handler::invalidate_folders($media_urls);

        return Action_Factory::invalidate_folders($media_urls, $post_action);
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_fav_channel_ids($plugin_cookies)
    {
        $fav_channel_ids = array();

        $favorites = $this->get_fav_cookie($plugin_cookies);
        if (isset($plugin_cookies->{$favorites})) {
            $fav_channel_ids = array_filter(explode(",", $plugin_cookies->{$favorites}));
        }

        return array_unique($fav_channel_ids);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @param array $ids
     */
    public function set_fav_channel_ids($plugin_cookies, $ids)
    {
        $plugin_cookies->{$this->get_fav_cookie($plugin_cookies)} = implode(',', array_unique($ids));
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_fav_cookie($plugin_cookies)
    {
        $channel_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : 'default';
        return 'favorite_channels_' . hash('crc32', $channel_list);
    }

    /**
     * @param $plugin_cookies
     * @throws Exception
     */
    public function ensure_channels_loaded(&$plugin_cookies)
    {
        if (!isset($this->channels)) {
            $this->load_channels($plugin_cookies);
        }
    }

    /**
     * @param $plugin_cookies
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $channels_list_path = '';
        if (!isset($plugin_cookies->pass_sex)) {
            $plugin_cookies->pass_sex = '0000';
        }

        try {
            $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
            $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;
            hd_print(__METHOD__ . ": Load channels list using source: $source");
            switch ($source) {
                case 1:
                    $channels_list_path = smb_tree::get_folder_info($plugin_cookies, PARAM_CH_LIST_PATH, get_install_path());
                    $channels_list_path .= $channels_list;
                    hd_print(__METHOD__ . ": load from: $channels_list_path");
                    break;
                case 2:
                    if (!empty($plugin_cookies->channels_url)) {
                        $url_path = $plugin_cookies->channels_url;
                    } else {
                        $url_path = $this->plugin->config->plugin_info['app_channels_url_path'];
                    }

                    if (substr($url_path, -1) !== '/') {
                        $url_path .= '/';
                    }

                    $url_path .= $channels_list;
                    hd_print(__METHOD__ . ": load folder link: $url_path");
                    break;
                case 3:
                    if (!empty($plugin_cookies->channels_direct_url)) {
                        $url_path = $plugin_cookies->channels_direct_url;
                    } else {
                        if (!isset($this->plugin->config->plugin_info['app_direct_links'][$channels_list])) {
                            throw new Exception(__METHOD__ . ": Direct link not set!");
                        }
                        $url_path = $this->plugin->config->plugin_info['app_direct_links'][$channels_list];
                    }

                    hd_print(__METHOD__ . ": load direct link: $url_path");
                    break;
            }

            if (!empty($url_path)) {
                try {
                    $channels_list_path = get_data_path(hash('crc32', $url_path));
                    if (is_file($channels_list_path)) {
                        unlink($channels_list_path);
                    }
                    file_put_contents($channels_list_path, HD::http_get_document($url_path));
                } catch (Exception $ex) {
                    if (!file_exists($channels_list_path)) {
                        hd_print(__METHOD__ . ": Can't fetch channel_list from $url_path " . $ex->getMessage());
                        return;
                    }
                }
            }

            file_put_contents(get_temp_path("current_list.xml"), file_get_contents($channels_list_path));
            $xml = HD::parse_xml_file($channels_list_path);
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Can't fetch channel_list $channels_list_path " . $ex->getMessage());
            return;
        }

        if ($xml->getName() !== 'tv_info') {
            hd_print(__METHOD__ . ": Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception(__METHOD__ . ": Invalid XML document");
        }

        $max_support_ch_list_ver = $this->plugin->config->plugin_info['app_ch_list_version'];
        if ($max_support_ch_list_ver < (int)$xml->vesion_info->list_version) {
            $message = TR::t('warn_msg1');
            hd_print($message);
            $this->plugin->config->set_last_error($message);
        }

        // read category
        $this->show_all_channels_group = !isset($plugin_cookies->show_all) || $plugin_cookies->show_all === 'yes';
        $fav_category_id = Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID;
        $groups = new Hashed_Array();
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = __METHOD__ . ": Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            if (isset($xml_tv_category->special_group)) {
                hd_print(__METHOD__ . ": special group $xml_tv_category->special_group, icon: $xml_tv_category->icon_url");
                switch ((string)$xml_tv_category->special_group) {
                    case Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID:
                        $fav_category_id = (int)$xml_tv_category->id;
                        $fav_group = new Favorites_Group(
                            Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID,
                            Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                            isset($xml_tv_category->icon_url) ? (string)$xml_tv_category->icon_url : Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH);
                        break;
                    case Default_Dune_Plugin::VOD_GROUP_ID:
                        $vod_group = new Vod_Group(
                            Default_Dune_Plugin::VOD_GROUP_ID,
                            Default_Dune_Plugin::VOD_GROUP_CAPTION,
                            isset($xml_tv_category->icon_url) ? (string)$xml_tv_category->icon_url : Default_Dune_Plugin::VOD_GROUP_ICON_PATH);
                        break;
                    case Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID:
                        $all_channels = new All_Channels_Group(
                            $this,
                            Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID,
                            Default_Dune_Plugin::ALL_CHANNEL_GROUP_CAPTION,
                            isset($xml_tv_category->icon_url) ? (string)$xml_tv_category->icon_url : Default_Dune_Plugin::ALL_CHANNEL_GROUP_ICON_PATH);
                        break;
                    case Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID:
                        $history_channels = new History_Group(
                            $this,
                            Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID,
                            Default_Dune_Plugin::PLAYBACK_HISTORY_CAPTION,
                            isset($xml_tv_category->icon_url) ? (string)$xml_tv_category->icon_url : Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ICON_PATH);
                        break;
                }
            } else if (!isset($xml_tv_category->disabled)) {
                //hd_print("regular group $xml_tv_category->caption, icon: $xml_tv_category->icon_url");
                $groups->put(new Default_Group(
                    (string)$xml_tv_category->id,
                    (string)$xml_tv_category->caption,
                    (string)$xml_tv_category->icon_url));
                hd_print(__METHOD__ . ": Added category: $xml_tv_category->caption");
            }
        }

        // build categories in correct order
        $this->groups = new Hashed_Array();

        // All channels category
        if (!isset($all_channels)) {
            hd_print(__METHOD__ . ": Using default all channels group, icon: " . Default_Dune_Plugin::ALL_CHANNEL_GROUP_ICON_PATH);
            $all_channels = new All_Channels_Group(
                $this,
                Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID,
                Default_Dune_Plugin::ALL_CHANNEL_GROUP_CAPTION,
                Default_Dune_Plugin::ALL_CHANNEL_GROUP_ICON_PATH);
        }
        $this->groups->put($all_channels);

        // Favorites group
        if ($this->is_favorites_supported()) {
            if (!isset($fav_group)) {
                hd_print(__METHOD__ . ": Using default favorites channels group icon: " . Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH);
                $fav_group = new Favorites_Group(
                    Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID,
                    Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                    Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH);
            }

            $this->groups->put($fav_group);
        }

        // History channels category
        if (!isset($history_channels)) {
            hd_print(__METHOD__ . ": Using default history channels group, icon: " . Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ICON_PATH);
            $history_channels = new History_Group(
                $this,
                Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID,
                Default_Dune_Plugin::PLAYBACK_HISTORY_CAPTION,
                Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ICON_PATH);
        }
        $this->groups->put($history_channels);

        // Vod group
        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            if (!isset($vod_group)) {
                hd_print(__METHOD__ . ": Using default vod channels group icon: " . Default_Dune_Plugin::VOD_GROUP_ICON_PATH);
                $vod_group = new Vod_Group(Default_Dune_Plugin::VOD_GROUP_ID,
                    Default_Dune_Plugin::VOD_GROUP_CAPTION,
                    Default_Dune_Plugin::VOD_GROUP_ICON_PATH);
            }
            $this->vod_group = $vod_group;
        }

        // All collected categories from channels list
        foreach ($groups as $group) {
            $this->groups->put($group);
        }

        $this->plugin->config->GetAccountInfo($plugin_cookies);
        $this->plugin->config->SetupM3uParser(true, $plugin_cookies);
        $pl_entries = $this->plugin->config->GetPlaylistStreamsInfo($plugin_cookies);

        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        // Read channels
        $this->epg_ids = array();
        $this->channels = new Hashed_Array();
        $num = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print(__METHOD__ . ": Error: unexpected node '{$xml_tv_channel->getName()}'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) {
                hd_print(__METHOD__ . ": Channel $xml_tv_channel->caption is disabled");
                continue;
            }

            // Read category id from channel
            if (!isset($xml_tv_channel->tv_category_id)) {
                hd_print(__METHOD__ . ": Error: Category undefined for channel $xml_tv_channel->caption !");
                continue;
            }

            // read channel id and custom url if exist
            if (isset($xml_tv_channel->channel_id)) {
                $channel_id = (string)$xml_tv_channel->channel_id;
                $ext_params = isset($pl_entries[$channel_id]) ? $pl_entries[$channel_id] : array();
                // update stream url by channel ID
                $hash = $channel_id;
                $streaming_url = isset($xml_tv_channel->streaming_url) ? (string)$xml_tv_channel->streaming_url : '';
            } else {
                // custom url, no id, play as is set channel id as url hash
                $streaming_url = (string)$xml_tv_channel->streaming_url;
                $hash = $channel_id = hash('crc32', $streaming_url);
                $ext_params = array();
            }

            $streaming_url_type = (!empty($streaming_url) && isset($xml_tv_channel->custom_url_type)) ? $xml_tv_channel->custom_url_type : '';

            // custom archive url or template
            $custom_archive_url = isset($xml_tv_channel->catchup_url_template) ? $xml_tv_channel->catchup_url_template : '';
            $custom_arc_url_type = isset($xml_tv_channel->custom_arc_url_type) ? $xml_tv_channel->custom_arc_url_type : '';

            $num++;
            $tv_category_id = (int)$xml_tv_channel->tv_category_id;
            if ($this->channels->has($hash)) {
                $channel = $this->channels->get($hash);
                if ($tv_category_id !== $fav_category_id) {
                    foreach($channel->get_groups() as $group) {
                        if ($group->get_id() !== $fav_category_id) {
                            hd_print(__METHOD__ . ": Channel $xml_tv_channel->caption ($channel_id) already exist in category: " . $group->get_title() . "(" . $group->get_id() . ")");
                        }
                    }
                }
            } else {
                $icon_url = (string)$xml_tv_channel->icon_url;
                $number = $num;

                $epg1 = (string)$xml_tv_channel->epg_id;
                $epg2 = (empty($xml_tv_channel->tvg_id)) ? $epg1 : (string)$xml_tv_channel->tvg_id;
                $protected = (int)$xml_tv_channel->protected && isset($plugin_cookies->pass_sex) && !empty($plugin_cookies->pass_sex);
                $channel = new Default_Channel(
                    $hash,
                    $channel_id,
                    (string)$xml_tv_channel->caption,
                    $icon_url,
                    $streaming_url,
                    $custom_archive_url,
                    $streaming_url_type,
                    $custom_arc_url_type,
                    (int)$xml_tv_channel->archive,
                    $number,
                    $epg1,
                    $epg2,
                    $protected,
                    (int)$xml_tv_channel->timeshift_hours,
                    $ext_params
                );
                $this->channels->put($channel);

                $this->epg_ids[$epg1] = '';
            }

            // Link group and channel.
            if (($tv_category_id === $fav_category_id || $xml_tv_channel->favorite) && $this->is_favorites_supported()) {
                // favorites category
                if (in_array($channel_id, $fav_channel_ids) === false) {
                    hd_print(__METHOD__ . ": Added from channels list to favorites channel $hash ($xml_tv_channel->caption)");
                    $fav_channel_ids[] = $channel_id;
                }
            } else if (!$this->groups->has($tv_category_id)) {
                // Category disabled or unknown
                hd_print(__METHOD__ . ": Unknown category $tv_category_id");
            } else {
                $group = $this->groups->get($tv_category_id);
                if (is_null($group))
                    hd_print(__METHOD__ . ": unknown group: $tv_category_id");
                $channel->add_group($group);
                $group->add_channel($channel);
            }
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

        hd_print(__METHOD__ . ": Loaded: channels: {$this->channels->size()}, groups: {$this->groups->size()}");

        if (isset($plugin_cookies->epg_source)
            && $plugin_cookies->epg_source === Plugin_Constants::EPG_INTERNAL
            && empty($this->plugin->config->epg_man->xmltv_data)) {

            $xmltv_idx = $this->plugin->config->epg_man->get_xmltv_idx($plugin_cookies);
            $cached_file = $this->plugin->config->epg_man->get_xml_cached_file($xmltv_idx, $plugin_cookies);
            if (!empty($cached_file)) {
                $max_cache_time = 3600 * 24 * (isset($plugin_cookies->epg_cache_ttl) ? $plugin_cookies->epg_cache_ttl : 3);
                hd_print(__METHOD__ . ": Checking: $cached_file ($xmltv_idx)");
                if (false !== Epg_Manager::is_xmltv_cache_valid($cached_file, $max_cache_time)) {
                    $this->plugin->config->epg_man->xmltv_data = Epg_Manager::load_xmltv_index($cached_file);
                } else {
                    $url = $this->plugin->config->epg_man->get_xmltv_url($xmltv_idx);
                    $res = Epg_Manager::download_xmltv_url($url, $cached_file);
                    if (true === $res) {
                        Epg_Manager::index_xmltv_file($cached_file, $this->epg_ids);
                    } else {
                        hd_print(__METHOD__ . ": $res");
                    }
                }
            }
        }
    }

    /**
     * @param User_Input_Handler $handler
     * @param $plugin_cookies
     * @return array
     */
    public function reload_channels(User_Input_Handler $handler, &$plugin_cookies)
    {
        hd_print(__METHOD__ . ": Reload channels");
        $this->plugin->config->ClearPlaylistCache($plugin_cookies);
        $this->plugin->config->ClearChannelsCache($plugin_cookies);
        $this->unload_channels();
        try {
            $this->load_channels($plugin_cookies);
        } catch (Exception $e) {
            hd_print(__METHOD__ . ": Reload channel list failed: $plugin_cookies->channels_list");
            return null;
        }

        Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
        $post_action = Starnet_Epfs_Handler::invalidate_folders(null,
            User_Input_Handler_Registry::create_action($handler, RESET_CONTROLS_ACTION_ID));

        return Action_Factory::invalidate_folders(array(
            Starnet_Tv_Groups_Screen::get_media_url_str(),
            Starnet_Tv_Channel_List_Screen::ID
        ), $post_action);
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
        hd_print(__METHOD__ . ": channel: $channel_id archive_ts: $archive_ts, protect code: $protect_code");

        try {
            $this->ensure_channels_loaded($plugin_cookies);

            $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
            // get channel by hash
            $channel = $this->get_channel($channel_id);
            if ($protect_code !== $pass_sex && $channel->is_protected()) {
                throw new Exception("Wrong adult password");
            }

            if (!$channel->is_protected()) {
                Playback_Points::push($channel_id, ($archive_ts !== -1 ? $archive_ts : ($channel->has_archive() ? time() : 0)));
            }

            // update url if play archive or different type of the stream
            $url = $this->plugin->config->GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);

            $zoom_data = HD::get_data_items(self::CHANNELS_ZOOM, true);
            if (isset($zoom_data[$channel_id])) {
                $zoom_preset = $zoom_data[$channel_id];
                hd_print(__METHOD__ . ": zoom_preset: $zoom_preset");
            } else if (!is_android() && !is_apk()) {
                $zoom_preset = DuneVideoZoomPresets::normal;
                hd_print(__METHOD__ . ": zoom_preset: reset to normal $zoom_preset");
            } else {
                $zoom_preset = '-';
                //hd_print(__METHOD__ . ": zoom_preset: not applicable");
            }

            if ($zoom_preset !== '-') {
                $url .= (strpos($url, "|||dune_params") === false ? "|||dune_params|||" : ",");
                $url .= "zoom:$zoom_preset";
            }

            hd_print(__METHOD__ . ": $url");
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Exception: " . $ex->getMessage());
            $url = '';
        }

        return $url;
    }

    /**
     * @param string $channel_id
     * @param integer $day_start_ts
     * @param $plugin_cookies
     * @return array
     */
    public function get_day_epg($channel_id, $day_start_ts, &$plugin_cookies)
    {
        $day_epg = array();

        try {
            // get channel by hash
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Can't get channel with ID: $channel_id");
            return $day_epg;
        }

        // correct day start to local timezone
        $day_start_ts -= get_local_time_zone_offset();

        //hd_print(__METHOD__ . ": day_start timestamp: $day_start_ts (" . format_datetime("Y-m-d H:i", $day_start_ts) . ")");
        $day_epg_items = $this->plugin->config->epg_man->get_day_epg_items($channel, $day_start_ts, $plugin_cookies);
        if ($day_epg_items !== false) {
            // get personal time shift for channel
            $time_shift = 3600 * ($channel->get_timeshift_hours() + (isset($plugin_cookies->epg_shift) ? $plugin_cookies->epg_shift : 0));
            //hd_print(__METHOD__ . ": EPG time shift $time_shift");
            foreach ($day_epg_items as $time => $value) {
                $tm_start = (int)$time + $time_shift;
                $tm_end = (int)$value[Epg_Params::EPG_END] + $time_shift;
                $day_epg[] = array
                (
                    PluginTvEpgProgram::start_tm_sec => $tm_start,
                    PluginTvEpgProgram::end_tm_sec => $tm_end,
                    PluginTvEpgProgram::name => $value[Epg_Params::EPG_NAME],
                    PluginTvEpgProgram::description => $value[Epg_Params::EPG_DESC],
                );

                //hd_print(format_datetime("m-d H:i", $tm_start) . " - " . format_datetime("m-d H:i", $tm_end) . " {$value[Epg_Params::EPG_NAME]}");
            }
        }

        return $day_epg;
    }

    public function get_program_info($channel_id, $program_ts, $plugin_cookies)
    {
        $program_ts = ($program_ts > 0 ? $program_ts : time());
        hd_print(__METHOD__ . ": for $channel_id at time $program_ts " . format_datetime("Y-m-d H:i", $program_ts));
        $day_start = date("Y-m-d", $program_ts);
        $day_ts = strtotime($day_start) + get_local_time_zone_offset();
        $day_epg = $this->get_day_epg($channel_id, $day_ts, $plugin_cookies);
        foreach ($day_epg as $item) {
            if ($program_ts >= $item[PluginTvEpgProgram::start_tm_sec] && $program_ts < $item[PluginTvEpgProgram::end_tm_sec]) {
                return $item;
            }
        }

        hd_print(__METHOD__ . ": No entries found for time $program_ts");
        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_tv_info(MediaURL $media_url, &$plugin_cookies)
    {
        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;

        //$t = microtime(1);

        $this->ensure_channels_loaded($plugin_cookies);
        $this->playback_runtime = PHP_INT_MAX;

        $channels = array();

        foreach ($this->get_channels() as $channel) {
            $group_id_arr = array();

            if ($this->show_all_channels_group === true) {
                $group_id_arr[] = Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID;
            }

            foreach ($channel->get_groups() as $g) {
                $group_id_arr[] = $g->get_id();
            }

            $channels[] = array(
                PluginTvChannel::id => $channel->get_id(),
                PluginTvChannel::caption => $channel->get_title(),
                PluginTvChannel::group_ids => $group_id_arr,
                PluginTvChannel::icon_url => $channel->get_icon_url(),
                PluginTvChannel::number => $channel->get_number(),

                PluginTvChannel::have_archive => $channel->has_archive(),
                PluginTvChannel::is_protected => $channel->is_protected(),

                // set default epg range
                PluginTvChannel::past_epg_days => $channel->get_past_epg_days(),
                PluginTvChannel::future_epg_days => $channel->get_future_epg_days(),

                PluginTvChannel::archive_past_sec => $channel->get_archive_past_sec(),
                PluginTvChannel::archive_delay_sec => (isset($plugin_cookies->delay_time) ? $plugin_cookies->delay_time : 60),

                // Buffering time
                PluginTvChannel::buffering_ms => (isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000),
                PluginTvChannel::timeshift_hours => $channel->get_timeshift_hours(),

                PluginTvChannel::playback_url_is_stream_url => $this->playback_url_is_stream_url,
            );
        }

        $groups = array();

        /** @var Default_Group $group */
        foreach ($this->get_groups() as $group) {
            if ($group->is_favorite_group()) {
                continue;
            }

            if ($group->is_history_group()) {
                continue;
            }

            if ($this->show_all_channels_group === false && $group->is_all_channels_group()) {
                continue;
            }

            $groups[] = array
            (
                PluginTvGroup::id => $group->get_id(),
                PluginTvGroup::caption => $group->get_title(),
                PluginTvGroup::icon_url => $group->get_icon_url()
            );
        }

        $is_favorite_group = isset($media_url->is_favorites);
        $initial_group_id = (string)$media_url->group_id;
        $initial_is_favorite = 0;

        if ($is_favorite_group) {
            $initial_group_id = null;
            $initial_is_favorite = 1;
        }

        $fav_channel_ids = null;
        if ($this->is_favorites_supported()) {
            $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);
        }

        //hd_print(__METHOD__ . ': Info loaded at ' . (microtime(1) - $t) . ' secs');

        return array(
            PluginTvInfo::show_group_channels_only => true,

            PluginTvInfo::groups => $groups,
            PluginTvInfo::channels => $channels,

            PluginTvInfo::favorites_supported => $this->is_favorites_supported(),
            PluginTvInfo::favorites_icon_url => Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH,

            PluginTvInfo::initial_channel_id => (string)$media_url->channel_id,
            PluginTvInfo::initial_group_id => $initial_group_id,

            PluginTvInfo::initial_is_favorite => $initial_is_favorite,
            PluginTvInfo::favorite_channel_ids => $fav_channel_ids,

            PluginTvInfo::initial_archive_tm => isset($media_url->archive_tm) ? (int)$media_url->archive_tm : -1,

            PluginTvInfo::epg_font_size => $epg_font_size,

            PluginTvInfo::actions => $this->get_action_map(),
            PluginTvInfo::timer => Action_Factory::timer(1000),
        );
    }

    public function get_action_map()
    {
        $actions = array();
        //$actions[GUI_EVENT_KEY_B_GREEN] = User_Input_Handler_Registry::create_action($this, ACTION_ZOOM_MENU);
        $actions[GUI_EVENT_PLAYBACK_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP);

        return $actions;
    }

    /**
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        if (!isset($user_input->control_id)) {
            return null;
        }

        $channel_id = $user_input->plugin_tv_channel_id;

        Playback_Points::update($channel_id);

        switch ($user_input->control_id) {
            case GUI_EVENT_PLAYBACK_STOP:
                if (!$this->plugin->new_ui_support
                    || !(isset($user_input->playback_stop_pressed) || isset($user_input->playback_power_off_needed))) break;

                Playback_Points::save(smb_tree::get_folder_info($plugin_cookies, PARAM_HISTORY_PATH));
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders(null,
                    Action_Factory::invalidate_folders(array(Starnet_TV_History_Screen::get_media_url_str())));

            case ACTION_ZOOM_MENU:
                $attrs['dialog_params']['frame_style'] = DIALOG_FRAME_STYLE_GLASS;
                $zoom_data = HD::get_data_items(self::CHANNELS_ZOOM, true);
                $dune_zoom = isset($zoom_data[$channel_id]) ? $zoom_data[$channel_id] : DuneVideoZoomPresets::not_set;

                $defs = array();
                Control_Factory::add_label($defs,'', TR::t('tv_screen_switch_channel'));
                Control_Factory::add_combobox($defs, $this, null, ACTION_ZOOM_SELECT, "",
                    $dune_zoom, DuneVideoZoomPresets::$zoom_ops, 1000, true);
                Control_Factory::add_button_close ($defs, $this, null,ACTION_ZOOM_APPLY,
                    "", TR::t('apply'), 600);
                return Action_Factory::show_dialog(TR::t('tv_screen_zoom_channel'), $defs,true,0, $attrs);

            case ACTION_ZOOM_APPLY:
                $zoom_data = HD::get_data_items(self::CHANNELS_ZOOM, true);
                if ($user_input->{ACTION_ZOOM_SELECT} === DuneVideoZoomPresets::not_set) {
                    $zoom_preset = DuneVideoZoomPresets::normal;
                    hd_print(__METHOD__ . ": Zoom preset removed for channel: $channel_id ($zoom_preset)");
                    unset ($zoom_data[$channel_id]);
                } else {
                    $zoom_preset = $zoom_data[$channel_id] = $user_input->{ACTION_ZOOM_SELECT};
                    hd_print(__METHOD__ . ": Zoom preset $zoom_preset for channel: $channel_id");
                }

                HD::put_data_items(self::CHANNELS_ZOOM, $zoom_data);
                //set_video_zoom(get_zoom_value($zoom_preset));
                break;
        }

        return null;
    }
}

<?php
require_once 'lib/tv/tv.php';
require_once 'lib/tv/default_channel.php';
require_once 'lib/tv/all_channels_group.php';
require_once 'lib/tv/favorites_group.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/hashed_array.php';
require_once 'lib/epg_xml_parser.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';


class Starnet_Tv implements Tv, User_Input_Handler
{
    const ID = 'tv';

    const MODE_CHANNELS_1_TO_N = false;
    const MODE_CHANNELS_N_TO_M = true;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @var Starnet_Plugin
     */
    protected $plugin;

    /**
     * @var Epg_Manager
     */
    protected $epg_man;

    /**
     * @var bool
     */
    protected $mode;

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
     * @template Group
     * @var Hashed_Array<Group>
     */
    protected $groups;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Starnet_Plugin $plugin
     */
    public function __construct(Starnet_Plugin $plugin)
    {
        $this->plugin = $plugin;
        $this->mode = self::MODE_CHANNELS_N_TO_M;
        $this->playback_url_is_stream_url = false;
        $this->epg_man = new Epg_Manager($plugin->config);

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
        $channel = $this->channels->get($channel_id);

        if (is_null($channel)) {
            hd_print("Unknown channel: $channel_id");
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
            hd_print("Unknown group: $group_id");
            HD::print_backtrace();
            throw new Exception("Unknown group: $group_id");
        }

        return $g;
    }

    /**
     * @param array &$items
     */
    public function add_special_groups(&$items)
    {
        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $items[] = array(
                PluginRegularFolderItem::media_url => MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Vod_Category_List_Screen::ID,
                        'name' => 'VOD',
                    )
                ),
                PluginRegularFolderItem::caption => Default_Dune_Plugin::VOD_GROUP_CAPTION,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => Default_Dune_Plugin::VOD_GROUP_ICON
                )
            );
        }
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
                    hd_print("Add channel $channel_id to favorites");
                    $fav_channel_ids[] = $channel_id;
                }
                break;
            case PLUGIN_FAVORITES_OP_REMOVE:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false) {
                    hd_print("Remove channel $channel_id from favorites");
                    unset ($fav_channel_ids[$k]);
                }
                break;
            case 'clear_favorites':
                hd_print("Clear favorites");
                $fav_channel_ids = array();
                break;
            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== 0) {
                    hd_print("Move channel $channel_id up");
                    $t = $fav_channel_ids[$k - 1];
                    $fav_channel_ids[$k - 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $k = array_search($channel_id, $fav_channel_ids);
                if ($k !== false && $k !== count($fav_channel_ids) - 1) {
                    hd_print("Move channel $channel_id down");
                    $t = $fav_channel_ids[$k + 1];
                    $fav_channel_ids[$k + 1] = $fav_channel_ids[$k];
                    $fav_channel_ids[$k] = $t;
                }
                break;
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

        $media_urls = array(Starnet_Tv_Favorites_Screen::get_media_url_str(), Starnet_Tv_Channel_List_Screen::get_media_url_str(Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID));

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
                        $url_path = $this->plugin->plugin_info['app_channels_url_path'];
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
            $this->groups->put(new Favorites_Group(
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH));
            $fav_ids = $this->get_fav_channel_ids($plugin_cookies);
            hd_print("Local Favorites: " . count($fav_ids));
        }

        // All channels group
        $this->groups->put(new All_Channels_Group(
            $this,
            Default_Dune_Plugin::ALL_CHANNEL_GROUP_CAPTION,
            Default_Dune_Plugin::ALL_CHANNEL_GROUP_ICON_PATH));

        // read category
        $fav_category_id = Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID;
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = "Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            if (isset($xml_tv_category->disabled)) {
                continue;
            }

            if (!isset($xml_tv_category->favorite)) {
                $this->groups->put(new Default_Group((string)$xml_tv_category->id,
                    (string)$xml_tv_category->caption,
                    (string)$xml_tv_category->icon_url));
                hd_print("Added category: $xml_tv_category->caption");
            } else if ($xml_tv_category->favorite == "true") {
                $fav_category_id = (int)$xml_tv_category->id;
                //hd_print("Embedded Favorites category: $xml_tv_category->caption ($fav_category_id)");
            }
        }

        $this->plugin->config->GetAccountInfo($plugin_cookies);
        $this->plugin->config->SetupM3uParser(true, $plugin_cookies);
        $pl_entries = $this->plugin->config->GetPlaylistStreamsInfo();

        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        // Read channels
        $num = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '{$xml_tv_channel->getName()}'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) {
                hd_print("Channel $xml_tv_channel->caption is disabled");
                continue;
            }

            // Read category id from channel
            if (!isset($xml_tv_channel->tv_category_id)) {
                hd_print("Error: Category undefined for channel $xml_tv_channel->caption !");
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
                    hd_print("$tv_category_id !== $fav_category_id");
                    hd_print("Channel $xml_tv_channel->caption ($channel_id) already exist in category $tv_category_id");
                }
            } else {
                // https not supported for old players
                // $icon_url = str_replace("https://", "http://", (string)$xml_tv_channel->icon_url);
                $icon_url = (string)$xml_tv_channel->icon_url;
                $number = isset($xml_tv_channel->int_id) ? (int)$xml_tv_channel->int_id : $num;

                $epg1 = (string)$xml_tv_channel->epg_id;
                $epg2 = (empty($xml_tv_channel->tvg_id)) ? $epg1 : (string)$xml_tv_channel->tvg_id;

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
                    (int)$xml_tv_channel->protected,
                    (int)$xml_tv_channel->timeshift_hours,
                    $ext_params
                );
                $this->channels->put($channel);
            }

            // Link group and channel.
            if (($tv_category_id === $fav_category_id || $xml_tv_channel->favorite) && $this->is_favorites_supported()) {
                // favorites category
                if (in_array($channel_id, $fav_channel_ids) === false) {
                    hd_print("Added from channels list to favorites channel $hash ($xml_tv_channel->caption)");
                    $fav_channel_ids[] = $channel_id;
                }
            } else if (!$this->groups->has($tv_category_id)) {
                // Category disabled or unknown
                hd_print("Unknown category $tv_category_id");
            } else {
                $group = $this->groups->get($tv_category_id);
                if (is_null($group))
                    hd_print("unknown group: $tv_category_id");
                $channel->add_group($group);
                $group->add_channel($channel);
            }
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

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
     * @throws Exception
     */
    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        hd_print("get_tv_playback_url: channel: $channel_id archive_ts: $archive_ts");
        $this->ensure_channels_loaded($plugin_cookies);

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

        if ($this->plugin->history_support && !$channel->is_protected()) {
            Playback_Points::push($channel_id, ($archive_ts !== -1 ? $archive_ts : ($channel->has_archive() ? time() : 0)));
        }

        // update url if play archive or different type of the stream
        $url = $this->plugin->config->GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
        hd_print("get_tv_playback_url: $url");
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
        try {
            // get channel by hash
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;
        $day_epg = $channel->get_day_epg_items($epg_source, $day_start_ts);
        if ($day_epg !== false)
            return $day_epg;

        try {
            $day_start_ts -= get_local_time_zone_offset();
            $epg = $this->epg_man->get_epg($channel, $epg_source, $day_start_ts, $plugin_cookies);
            if (count($epg) === 0) {
                hd_print("No data from $epg_source EPG for $channel_id");
                return array();
            }
        } catch (Exception $ex) {
            hd_print("Can't fetch EPG ID from $epg_source epg source: " . $ex->getMessage());
            return array();
        }

        hd_print("Loaded " . count($epg) . " EPG entries");
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;
        $day_epg = array();
        foreach ($epg as $time => $value) {
            $day_epg[] = array
            (
                PluginTvEpgProgram::start_tm_sec => (int)$time + $time_shift,
                PluginTvEpgProgram::end_tm_sec => (int)$value[Epg_Params::EPG_END] + $time_shift,
                PluginTvEpgProgram::name => $value[Epg_Params::EPG_NAME],
                PluginTvEpgProgram::description => $value[Epg_Params::EPG_DESC],
            );
        }

        $channel->set_day_epg_items($epg_source, $day_start_ts, $day_epg);
        $this->set_channel($channel);

        return $day_epg;
    }

    public function get_program_info($channel_id, $program_ts, $plugin_cookies)
    {
        hd_print("Starnet_Tv::get_program_info for $channel_id at time $program_ts");
        $day_ts = strtotime(date('d-M-Y', ($program_ts > 0 ? $program_ts : time())));
        $day_epg = $this->get_day_epg($channel_id, $day_ts, $plugin_cookies);
        foreach ($day_epg as $item) {
            if ($program_ts >= $item[PluginTvEpgProgram::start_tm_sec] && $program_ts < $item[PluginTvEpgProgram::end_tm_sec]) {
                return $item;
            }
        }

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

        $t = microtime(1);

        $this->ensure_channels_loaded($plugin_cookies);
        $this->playback_runtime = PHP_INT_MAX;

        $channels = array();

        foreach ($this->get_channels() as $channel) {
            $group_id_arr = array();

            if ($this->mode === self::MODE_CHANNELS_N_TO_M) {
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
                PluginTvChannel::archive_delay_sec => $channel->get_archive_delay_sec(),

                // Buffering time
                PluginTvChannel::buffering_ms => (isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000),
                PluginTvChannel::timeshift_hours => $channel->get_timeshift_hours(),

                PluginTvChannel::playback_url_is_stream_url => $this->playback_url_is_stream_url,
            );
        }

        $groups = array();

        foreach ($this->get_groups() as $g) {
            if ($g->is_favorite_group()) {
                continue;
            }

            if ($this->mode === self::MODE_CHANNELS_1_TO_N && $g->is_all_channels()) {
                continue;
            }

            $groups[] = array
            (
                PluginTvGroup::id => $g->get_id(),
                PluginTvGroup::caption => $g->get_title(),
                PluginTvGroup::icon_url => $g->get_icon_url()
            );
        }

        $is_favorite_group = isset($media_url->is_favorites);
        $initial_group_id = (string)$media_url->group_id;
        $initial_is_favorite = 0;

        if ($is_favorite_group) {
            $initial_group_id = null;
            $initial_is_favorite = 1;
        }

        if ($this->mode === self::MODE_CHANNELS_1_TO_N &&
            $initial_group_id === Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID) {
            $initial_group_id = null;
        }

        $fav_channel_ids = null;
        if ($this->is_favorites_supported()) {
            $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);
        }

        if ($this->plugin->history_support) {
            Playback_Points::init();
        }

        hd_print('TV Info loaded at ' . (microtime(1) - $t) . ' secs');

        return array(
            PluginTvInfo::show_group_channels_only => $this->mode,

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
        if ($this->plugin->new_ui_support) {
            $actions[GUI_EVENT_PLAYBACK_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP);
        }

        // dummy action. If actions empty plugin crashed
        $actions[GUI_EVENT_GOING_TO_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_GOING_TO_STOP);

        return $actions;
    }

    /**
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Starnet_Tv: handle_user_input');
        foreach ($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->control_id)) {
            return null;
        }

        if ($user_input->control_id === GUI_EVENT_PLAYBACK_STOP
            && (isset($user_input->playback_stop_pressed) || isset($user_input->playback_power_off_needed))) {

            if ($this->plugin->history_support && isset($user_input->plugin_tv_channel_id)) {
                Playback_Points::update($user_input->plugin_tv_channel_id);
            }

            Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
            return Starnet_Epfs_Handler::invalidate_folders();
        }

        return null;
    }
}

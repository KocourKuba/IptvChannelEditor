<?php
///////////////////////////////////////////////////////////////////////////

require_once 'tr.php';
require_once 'mediaurl.php';
require_once 'ordered_array.php';
require_once 'user_input_handler_registry.php';
require_once 'action_factory.php';
require_once 'control_factory.php';
require_once 'control_factory_ext.php';
require_once 'plugin_constants.php';
require_once 'plugin_macros.php';
require_once 'lib/cache_parameters.php';
require_once 'lib/epg/epg_manager_xmltv.php';
require_once 'lib/epg/epg_manager_json.php';
require_once 'lib/epg/ext_epg_program.php';

class Default_Dune_Plugin implements DunePlugin
{
    const SANDWICH_BASE = 'gui_skin://special_icons/sandwich_base.aai';
    const SANDWICH_MASK = 'cut_icon://{name=sandwich_mask}';
    const SANDWICH_COVER = 'cut_icon://{name=sandwich_cover}';
    const AUTHOR_LOGO = "IPTV Channel Editor by sharky72            ";

    /////////////////////////////////////////////////////////////////////////////
    // views variables
    const TV_SANDWICH_WIDTH = 245;
    const TV_SANDWICH_HEIGHT = 140;

    const TV_SANDWICH_WIDTH_SMALL = 160;
    const TV_SANDWICH_HEIGHT_SMALL = 160;

    const VOD_SANDWICH_WIDTH = 190;
    const VOD_SANDWICH_HEIGHT = 290;

    const VOD_CHANNEL_ICON_WIDTH = 190;
    const VOD_CHANNEL_ICON_HEIGHT = 290;

    const DEFAULT_MOV_ICON_PATH = 'plugin_file://img/mov_unset.png';
    const VOD_ICON_PATH = 'gui_skin://small_icons/movie.aai';
    const RESOURCE_URL = 'http://iptv.esalecrm.net/res';
    const HISTORY_FOLDER = 'history/';
    const PARSE_CONFIG = "%s_parse_config.json";

    /**
     * @var bool
     */
    public $new_ui_support;

    /**
     * @var array
     */
    protected $screens_views;

    /**
     * @var Starnet_Tv
     */
    public $tv;

    /**
     * @var Starnet_Vod
     */
    public $vod;

    /**
     * @var default_config
     */
    public $config;

    /**
     * @var array|Screen[]
     */
    private $screens;

    /**
     * @var M3uParser
     */
    protected $m3u_parser;

    /**
     * @var Epg_Manager_Xmltv|Epg_Manager_Json
     */
    protected $epg_manager;

    /**
     * @var array
     */
    protected $parameters;

    /**
     * @var bool
     */
    protected $postpone_save = false;

    /**
     * @var bool
     */
    protected $is_durty = false;

    /**
     * @var array
     */
    protected $credentials;

    /**
     * @var Ordered_Array
     */
    protected $favorite_ids;

    /**
     * @var Ordered_Array
     */
    protected $favorite_channel_list_ids;

    /**
     * @var Playback_Points
     */
    protected $playback_points;

    /**
     * @var bool
     */
    protected $ext_epg_supported = false;

    /**
     * @var bool
     */
    protected $need_update_epfs = false;

    protected function __construct()
    {
        ini_set('memory_limit', '256M');

        $this->screens = array();
        $this->new_ui_support = HD::rows_api_support();
        $this->m3u_parser = new M3uParser();
    }

    /**
     * set base plugin info from dune_plugin.xml
     * @throws Exception
     */
    public function plugin_setup()
    {
        set_debug_log(true);
        $plugin_info = get_plugin_manifest_info();

        $plugin_config_class = $plugin_info['app_class_name'];

        if (!class_exists($plugin_config_class)) {
            hd_debug_print("Unknown plugin: $plugin_config_class");
            throw new Exception("Unknown plugin type: $plugin_config_class");
        }

        if (!is_subclass_of($plugin_config_class, 'dynamic_config')) {
            hd_debug_print("plugin: $plugin_config_class not a subclass of 'dynamic_config'");
            throw new Exception("Wrong subclass: $plugin_config_class");
        }

        hd_debug_print("Instantiate class: $plugin_config_class");
        $this->config = new $plugin_config_class;
        $this->config->plugin_info = $plugin_info;
        $this->config->set_plugin($this);
        $this->config->init_defaults();
        $this->config->load_config();
        $this->config->load_embedded_account();
        $this->config->init_custom_config();

        $this->init_plugin();

        print_sysinfo();

        hd_debug_print_separator();
        hd_print("Plugin ID:           " . $plugin_info['app_type_name']);
        hd_print("Plugin name:         " . $plugin_info['app_caption']);
        hd_print("Plugin version:      " . $plugin_info['app_version']);
        hd_print("Plugin date:         " . $plugin_info['app_release_date']);
        hd_print("Account type:        " . $this->config->get_feature(Plugin_Constants::ACCESS_TYPE));
        hd_print("VOD page:            " . ($this->config->get_feature(Plugin_Constants::VOD_ENGINE) !== "None" ? "yes" : "no"));
        hd_print("LocalTime            " . format_datetime('Y-m-d H:i', time()));
        hd_print("TimeZone             " . getTimeZone());
        hd_print("Daylight             " . (date('I') ? 'yes' : 'no'));
        hd_print("Icon                 " . $plugin_info['app_logo']);
        hd_print("Background           " . $plugin_info['app_background']);
        hd_print("New UI support       " . ($this->new_ui_support ? "yes" : "no"));
        hd_print("Ext EPG support:     " . var_export(is_ext_epg_supported(), true));
        hd_print("Max ch. list version " . $plugin_info['app_ch_list_version']);

        if (!empty($plugin_info['app_update_path'])) {
            hd_print("Web update url       " . $plugin_info['app_update_path']);
        }

        if (!empty($plugin_info['app_channels_url_path'])) {
            hd_print("Channels path        " . $plugin_info['app_channels_url_path']);
        }

        if (!empty($this->config->plugin_info['app_direct_links'])) {
            foreach ($this->config->plugin_info['app_direct_links'] as $item) {
                hd_print("Channels direct link " . $item);
            }
        }
        hd_debug_print_separator();
    }

    public function create_setup_header(&$defs)
    {
        Control_Factory::add_vgap($defs, -10);
        Control_Factory::add_label($defs, self::AUTHOR_LOGO,
            " v.{$this->config->plugin_info['app_version']} [{$this->config->plugin_info['app_release_date']}]",
            20);
    }

    ///////////////////////////////////////////////////////////////////////

    public function init_plugin()
    {
        hd_debug_print_separator();
        // small hack to show parameters in log
        set_debug_log(true);
        $this->load(true);
        $this->update_log_level();

        $this->playback_points = new Playback_Points($this);
        $this->favorite_channel_list_ids = new Ordered_Array();

        $this->init_epg_manager();
        $this->create_screen_views();

        hd_debug_print("Init plugin done!");
        hd_debug_print_separator();
    }

    /**
     * @return M3uParser
     */
    public function get_m3u_parser()
    {
        return $this->m3u_parser;
    }

    /**
     * @return Epg_Manager_Xmltv|Epg_Manager_Json
     */
    public function get_epg_manager()
    {
        return $this->epg_manager;
    }

    /**
     * @return Playback_Points
     */
    public function &get_playback_points()
    {
        return $this->playback_points;
    }

    /**
     * @param $object
     * @return void
     */
    public function create_screen($object)
    {
        if (!is_null($object) && method_exists($object, 'get_id')) {
            $this->add_screen($object);
            User_Input_Handler_Registry::get_instance()->register_handler($object);
        } else {
            hd_debug_print(get_class($object) . ": Screen class is illegal. get_id method not defined!");
        }
    }

    public function is_json_capable()
    {
        $epg_url1 = $this->config->get_epg_param(Plugin_Constants::EPG_FIRST, Epg_Params::EPG_URL);
        $epg_url2 = $this->config->get_epg_param(Plugin_Constants::EPG_SECOND, Epg_Params::EPG_URL);
        return !empty($epg_url1) || !empty($epg_url2);
    }

    public function is_ext_epg_exist()
    {
        return $this->ext_epg_supported;
    }

    public function init_epg_manager()
    {
        $this->epg_manager = null;
        $engine = $this->get_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_JSON);
        hd_debug_print("Selected Epg_Manager: $engine");
        if ($engine === ENGINE_JSON) {
            if ($this->is_json_capable()) {
                hd_debug_print("Using 'Epg_Manager_Json' cache engine");
                $this->epg_manager = new Epg_Manager_Json($this);
            } else {
                $this->set_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_XMLTV);
            }
        }

        if (is_null($this->epg_manager)) {
            hd_debug_print("Using 'Epg_Manager_Xmltv' cache engine");
            $this->epg_manager = new Epg_Manager_Xmltv($this);
        }

        $this->epg_manager->init_indexer($this->get_cache_dir());
        $this->epg_manager->get_indexer()->clear_stalled_locks();
    }

    /**
     * @return string
     */
    public function get_cache_dir()
    {
        $cache_dir = smb_tree::get_folder_info($this->get_parameter(PARAM_CACHE_PATH));
        if (!is_null($cache_dir) && rtrim($cache_dir, DIRECTORY_SEPARATOR) === get_data_path(EPG_CACHE_SUBDIR)) {
            $this->remove_parameter(PARAM_CACHE_PATH);
            $cache_dir = null;
        }

        if (is_null($cache_dir)) {
            $cache_dir = get_data_path(EPG_CACHE_SUBDIR);
        }

        return $cache_dir;
    }

    /**
     * get all xmltv source
     *
     * @return Hashed_Array<string, Cache_Parameters> const
     */
    public function get_all_xmltv_sources()
    {
        hd_debug_print(null, true);

        /** @var Hashed_Array<string, float> $cache_params */
        $cache_params = $this->get_parameter(PARAM_EPG_CACHE_PARAMETERS, new Hashed_Array());

        /** @var Hashed_Array $sources */
        $xmltv_sources = new Hashed_Array();
        $ext_sources = $this->config->get_feature(Plugin_Constants::EPG_CUSTOM_SOURCE);
        if (!empty($ext_sources)) {
            hd_debug_print("ext xmltv sources: " . json_encode($ext_sources), true);
            foreach ($ext_sources as $key => $source) {
                $cached_type = new Cache_Parameters();
                $cached_type->url = $source['name'];
                $hash = Hashed_Array::hash($source['name']);
                $cached_type->ttl = $cache_params->has($hash) ? $cache_params->get($hash) : -1;
                $xmltv_sources->set($hash, $cached_type);
                hd_debug_print("$key => $source", true);
            }
        }

        return $xmltv_sources;
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Screen support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Screen $scr
     */
    protected function add_screen(Screen $scr)
    {
        if (isset($this->screens[$scr->get_id()])) {
            hd_debug_print("Error: screen (id: " . $scr->get_id() . ") already registered.");
        } else {
            $this->screens[$scr->get_id()] = $scr;
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $screen_id
     * @return Screen
     * @throws Exception
     */
    protected function get_screen_by_id($screen_id)
    {
        if (isset($this->screens[$screen_id])) {
            // hd_debug_print("'$screen_id'");
            return $this->screens[$screen_id];
        }

        hd_debug_print("Error: no screen with id '$screen_id' found.");
        print_backtrace();
        throw new Exception('Screen not found');
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @return Screen
     * @throws Exception
     */
    protected function get_screen_by_url(MediaURL $media_url)
    {
        $screen_id = isset($media_url->screen_id) ? $media_url->screen_id : $media_url->get_raw_string();

        return $this->get_screen_by_id($screen_id);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Folder support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_folder_view($media_url, &$plugin_cookies)
    {
        //hd_debug_print("MediaUrl: $media_url");
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_next_folder_view($media_url, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_next_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_regular_folder_items($media_url, $from_ndx, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_folder_range($decoded_media_url, $from_ndx, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // IPTV channels support (TV support).
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_tv_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_debug_print("TV is not supported");
            print_backtrace();
            return array();
        }

        $decoded_media_url = MediaURL::decode($media_url);

        return $this->tv->get_tv_info($decoded_media_url);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_debug_print("TV is not supported");
            print_backtrace();
            return '';
        }

        return $media_url;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param int $archive_tm_sec
     * @param string $protect_code
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_debug_print("TV is not supported");
            print_backtrace();
            return '';
        }

        return $this->tv->get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param integer $day_start_tm_sec
     * @param $plugin_cookies
     * @return array
     */
    public function get_day_epg($channel_id, $day_start_tm_sec, &$plugin_cookies)
    {
        hd_debug_print(null, true);

        $day_epg = array();
        try {
            if (is_null($this->tv)) {
                hd_debug_print("TV is not supported");
                print_backtrace();
                throw new Exception('TV is not supported');
            }

            // get channel by hash
            $channel = $this->tv->get_channel($channel_id);
            if ($channel === null) {
                throw new Exception("Undefined channel!");
            }

            // correct day start to local timezone
            $day_start_tm_sec -= get_local_time_zone_offset();

            // get personal time shift for channel
            $time_shift = 3600 * ($channel->get_timeshift_hours() + $this->get_parameter(PARAM_EPG_SHIFT, 0)) + $channel->get_timeshift_hours() * 60;
            hd_debug_print("EPG time shift $time_shift", true);
            $day_start_tm_sec += $time_shift;

            $show_ext_epg = $this->get_bool_parameter(PARAM_SHOW_EXT_EPG) && $this->ext_epg_supported;

            if (LogSeverity::$is_debug) {
                hd_debug_print("day_start timestamp: $day_start_tm_sec (" . format_datetime("Y-m-d H:i", $day_start_tm_sec) . ")");
            }

            foreach ($this->epg_manager->get_day_epg_items($channel, $day_start_tm_sec) as $time => $value) {
                $tm_start = (int)$time + $time_shift;
                $tm_end = (int)$value[Epg_Params::EPG_END] + $time_shift;
                $day_epg[] = array(
                    PluginTvEpgProgram::start_tm_sec => $tm_start,
                    PluginTvEpgProgram::end_tm_sec => $tm_end,
                    PluginTvEpgProgram::name => $value[Epg_Params::EPG_NAME],
                    PluginTvEpgProgram::description => $value[Epg_Params::EPG_DESC],
                );
                if (!$show_ext_epg || in_array($channel_id, $this->epg_manager->get_delayed_epg())) continue;

                $ext_epg[$time]["start_tm"] = $tm_start;
                $ext_epg[$time]["title"] = $value[Epg_Params::EPG_NAME];
                $ext_epg[$time]["desc"] = $value[Epg_Params::EPG_DESC];

                if (empty($value[PluginTvEpgProgram::icon_url])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::main_icon] = $channel->get_icon_url();
                } else {
                    $ext_epg[$time][PluginTvExtEpgProgram::main_icon] = $value[PluginTvEpgProgram::icon_url];
                }

                if (!empty($value[PluginTvExtEpgProgram::main_category])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::main_category] = $value[PluginTvExtEpgProgram::main_category];
                }

                if (!empty($value[PluginTvExtEpgProgram::icon_urls])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::icon_urls] = $value[PluginTvExtEpgProgram::icon_urls];
                }

                if (!empty($value[PluginTvExtEpgProgram::year])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::year] = $value[PluginTvExtEpgProgram::year];
                }

                if (!empty($value[PluginTvExtEpgProgram::country])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::country] = $value[PluginTvExtEpgProgram::country];
                }

                if (!empty($time[PluginTvExtEpgProgram::director])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::director] = $value[PluginTvExtEpgProgram::director];
                }

                if (!empty($value[PluginTvExtEpgProgram::composer])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::composer] = $value[PluginTvExtEpgProgram::composer];
                }

                if (!empty($value[PluginTvExtEpgProgram::editor])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::editor] = $value[PluginTvExtEpgProgram::editor];
                }

                if (!empty($value[PluginTvExtEpgProgram::writer])) {
                    $ext_epg[$time][PluginTvExtEpgProgram::writer] = $value[PluginTvExtEpgProgram::writer];
                }

                if (!empty($value[PluginTvExtEpgProgram::actor]))
                    $ext_epg[$time][PluginTvExtEpgProgram::actor] = $value[PluginTvExtEpgProgram::actor];

                if (!empty($value[PluginTvExtEpgProgram::presenter]))
                    $ext_epg[$time][PluginTvExtEpgProgram::presenter] = $value[PluginTvExtEpgProgram::presenter];

                if (!empty($value[PluginTvExtEpgProgram::imdb_rating]))
                    $ext_epg[$time][PluginTvExtEpgProgram::imdb_rating] = $value[PluginTvExtEpgProgram::imdb_rating];
            }
            $apk_subst = getenv('FS_PREFIX');
            $app_name = $this->config->plugin_info['app_type_name'];
            $dir = "$apk_subst/tmp/ext_epg";
            if (!empty($ext_epg) && create_path($dir)) {
                $filename = sprintf("%s-%s-%s.json", $app_name, Hashed_Array::hash($channel_id), strftime('%Y-%m-%d', $day_start_tm_sec));
                hd_debug_print("save ext_epg to: $filename");
                if (file_put_contents(get_temp_path($filename), pretty_json_format($ext_epg))) {
                    rename(get_temp_path($filename), "$dir/$filename");
                }
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
        }

        return $day_epg;
    }

    public function get_program_info($channel_id, $program_ts, $plugin_cookies)
    {
        $program_ts = ($program_ts > 0 ? $program_ts : time());
        hd_debug_print("for $channel_id at time $program_ts " . format_datetime("Y-m-d H:i", $program_ts));
        $day_start = date("Y-m-d", $program_ts);
        $day_ts = strtotime($day_start) + get_local_time_zone_offset();
        $day_epg = $this->get_day_epg($channel_id, $day_ts, $plugin_cookies);
        foreach ($day_epg as $item) {
            if ($program_ts >= $item[PluginTvEpgProgram::start_tm_sec] && $program_ts < $item[PluginTvEpgProgram::end_tm_sec]) {
                return $item;
            }
        }

        hd_debug_print("No entries found for time $program_ts");
        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @override DunePlugin
     * @param string $op_type
     * @param string $channel_id
     * @param $plugin_cookies
     * @return array
     */
    public function change_tv_favorites($op_type, $channel_id, &$plugin_cookies = null)
    {
        hd_debug_print(null, true);

        if (is_null($this->tv)) {
            hd_debug_print("TV is not supported");
            print_backtrace();
            return array();
        }

        $favorites = &$this->get_favorites();
        switch ($op_type) {

            case PLUGIN_FAVORITES_OP_ADD:
                if ($favorites->add_item($channel_id)) {
                    hd_debug_print("Add channel $channel_id to favorites", true);
                }
                break;

            case PLUGIN_FAVORITES_OP_REMOVE:
                if ($favorites->remove_item($channel_id)) {
                    hd_debug_print("Remove channel $channel_id from favorites", true);
                }
                break;

            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $favorites->arrange_item($channel_id, Ordered_Array::UP);
                hd_debug_print("Move channel $channel_id up", true);
                break;

            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $favorites->arrange_item($channel_id, Ordered_Array::DOWN);
                hd_debug_print("Move channel $channel_id down", true);
                break;

            case ACTION_ITEMS_CLEAR:
                hd_debug_print("Clear favorites", true);
                $favorites->clear();
                break;
        }

        $this->save_favorites();
        $this->set_need_update_epfs();
        return Starnet_Epfs_Handler::epfs_invalidate_folders(array(
                Starnet_Tv_Favorites_Screen::get_media_url_string(FAVORITES_GROUP_ID),
                Starnet_Tv_Channel_List_Screen::get_media_url_string(ALL_CHANNEL_GROUP_ID))
        );
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);

        return User_Input_Handler_Registry::get_instance()->handle_user_input($user_input, $plugin_cookies);
    }

    /**
     * @param bool $is_durty
     * @return void
     */
    public function set_need_update_epfs($is_durty = true)
    {
        $this->need_update_epfs = $is_durty;
    }

    /**
     * @param $plugin_cookies
     * @param array|null $media_urls
     * @param array|null $post_action
     * @param bool $all_except
     * @return array
     */
    public function invalidate_epfs_folders($plugin_cookies, $media_urls = null, $post_action = null, $all_except = false)
    {
        hd_debug_print(null, true);

        if ($this->need_update_epfs) {
            $this->need_update_epfs = false;
            Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
        }
        return Action_Factory::invalidate_folders($media_urls, $post_action, $all_except);
    }

    /**
     * @param $plugin_cookies
     * @param array|null $action
     * @param array|null $media_urls
     * @param array|null $post_action
     * @return array
     */
    public function update_invalidate_epfs_folders($plugin_cookies, $action, $media_urls = null, $post_action = null)
    {
        hd_debug_print(null, true);

        if ($this->need_update_epfs) {
            $this->need_update_epfs = false;
            Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
        }

        return Action_Factory::update_invalidate_folders(
            Action_Factory::update_invalidate_folders($action, Starnet_Tv_Rows_Screen::ID),
            $media_urls,
            $post_action
        );
    }
    ///////////////////////////////////////////////////////////////////////
    //
    // VOD support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_vod_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_debug_print("VOD is not supported");
            print_backtrace();
            return array();
        }

        return $this->vod->get_vod_info(MediaURL::decode($media_url));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print("VOD is not supported");
            return '';
        }

        return $media_url;
    }

    ///////////////////////////////////////////////////////////////////////
    // Parameters storage methods

    /**
     * load plugin/playlist settings
     *
     * @param bool $force
     * @return void
     */
    public function load($force = false)
    {
        if ($force) {
            $this->parameters = null;
        }

        if (is_null($this->parameters)) {
            hd_debug_print("Load: common.settings", true);
            $this->parameters = HD::get_data_items("common.settings", true, false);
            if (LogSeverity::$is_debug) {
                foreach ($this->parameters as $key => $param) hd_debug_print("$key => $param");
            }
        }
    }

    /**
     * save plugin/playlist settings
     *
     * @return void
     */
    public function save()
    {
        hd_debug_print(null, true);

        if (is_null($this->parameters)) {
            hd_debug_print("this->parameters is not set!", true);
        } else if (!$this->postpone_save) {
            $this->is_durty = false;
            hd_debug_print("Save: common.settings", true);
            foreach ($this->parameters as $key => $param) hd_debug_print("$key => $param", true);
            HD::put_data_items("common.settings", $this->parameters, false);
        }
    }

    /**
     * Get plugin parameters
     * Parameters does not depend on playlists and used globally
     *
     * @param string $type
     * @param mixed|null $default
     * @return mixed
     */
    public function get_parameter($type, $default = null)
    {
        $this->load();

        if (!isset($this->parameters[$type])) {
            if ($default === null) {
                return null;
            }
            $this->parameters[$type] = $default;
        }

        return $this->parameters[$type];
    }

    /**
     * set plugin parameters
     *
     * @param string $type
     * @param mixed $val
     */
    public function set_parameter($type, $val)
    {
        hd_debug_print(null, true);
        $this->parameters[$type] = $val;
        $this->is_durty = true;
        $this->save();
    }

    /**
     * Get plugin boolean parameters
     *
     * @param string $type
     * @param bool $default
     * @return bool
     */
    public function get_bool_parameter($type, $default = true)
    {
        $val = $this->get_parameter($type, $default ? SwitchOnOff::on : SwitchOnOff::off);
        return $val === SwitchOnOff::on;
    }

    /**
     * Set plugin boolean parameters
     *
     * @param string $type
     * @param bool $val
     */
    public function set_bool_parameter($type, $val)
    {
        $this->set_parameter($type, $val ? SwitchOnOff::on : SwitchOnOff::off);
    }

    /**
     * Remove parameter
     * @param string $type
     */
    public function remove_parameter($type)
    {
        unset($this->parameters[$type]);
        $this->save();
    }

    /**
     * @param string $param
     * @param bool $default
     * @return bool
     */
    public function toggle_parameter($param, $default = true)
    {
        $new_val = !$this->get_bool_parameter($param, $default);
        $this->set_bool_parameter($param, $new_val);
        return $new_val;
    }

    /**
     * Get plugin parameters
     * Parameters does not depend on playlists and used globally
     *
     * @param string $type
     * @return mixed
     */
    public function get_credentials($type)
    {
        if (is_null($this->credentials)) {
            hd_debug_print("Load: credentials", true);
            $this->credentials = HD::get_data_items("credentials.settings");
        }

        return isset($this->credentials[$type]) ? $this->credentials[$type] : '';
    }

    /**
     * set plugin parameters
     *
     * @param string $type
     * @param string $val
     */
    public function set_credentials($type, $val)
    {
        if (is_null($this->credentials)) {
            $this->credentials = array();
        }

        $this->credentials[$type] = $val;
        hd_debug_print("Save: credentials.settings", true);
        HD::put_data_items("credentials.settings", $this->credentials);
    }

    /**
     * @param $plugin_cookies
     * @return void
     */
    public function upgrade_old_settings(&$plugin_cookies)
    {
        hd_debug_print("upgrade old settings");
        $creds = array(
            'subdomain' => Ext_Params::M_SUBDOMAIN,
            'ott_key' => Ext_Params::M_OTT_KEY,
            'mediateka'=> Ext_Params::M_VPORTAL,
            'login' => Ext_Params::M_LOGIN,
            'password' => Ext_Params::M_PASSWORD,
            );

        foreach ($creds as $key => $value) {
            if (isset($plugin_cookies->{$key})) {
                $this->set_credentials($value, $plugin_cookies->{$key});
                unset($plugin_cookies->{$key});
            }
        }

        $params = array(
            PARAM_HISTORY_PATH => PARAM_HISTORY_PATH,
            PARAM_USE_UPDATER_PROXY => PARAM_USE_UPDATER_PROXY,
            PARAM_STREAM_FORMAT => PARAM_STREAM_FORMAT,
            PARAM_CACHE_PATH => PARAM_CACHE_PATH,
            PARAM_EPG_CACHE_TTL => PARAM_EPG_CACHE_TTL,
            PARAM_EPG_SHIFT => PARAM_EPG_SHIFT,
            PARAM_CHANNELS_SOURCE => PARAM_CHANNELS_SOURCE,
            PARAM_SHOW_ALL => PARAM_SHOW_ALL,
            PARAM_SHOW_FAVORITES => PARAM_SHOW_FAVORITES,
            PARAM_SHOW_HISTORY => PARAM_SHOW_HISTORY,
            PARAM_VOD_LAST => PARAM_VOD_LAST,
            PARAM_PLAYLIST_IDX => PARAM_PLAYLIST_IDX,
            PARAM_CHANNELS_LIST_PATH => PARAM_CHANNELS_LIST_PATH,
            PARAM_VOD_DEFAULT_QUALITY => PARAM_VOD_DEFAULT_QUALITY,
            'channels_list' => PARAM_CHANNELS_LIST_NAME,
            'buf_time' => PARAM_BUFFERING_TIME,
            'delay_time' => PARAM_ARCHIVE_DELAY_TIME,
            'channels_url' => PARAM_CHANNELS_URL,
            'channels_direct_url' => PARAM_CHANNELS_DIRECT_URL,
            'pass_sex' => PARAM_ADULT_PASSWORD,
            'server_id' => Ext_Params::M_SERVER_ID,
            'quality_id'=> Ext_Params::M_QUALITY_ID,
            'device_id' => Ext_Params::M_DEVICE_ID,
            'profile_id' => Ext_Params::M_PROFILE_ID,
        );

        foreach ($params as $key => $value) {
            if (isset($plugin_cookies->{$key})) {
                hd_debug_print("$key to $value");
                $this->set_parameter($value, $plugin_cookies->{$key});
                unset($plugin_cookies->{$key});
            }
        }

        $channel_list = $this->get_parameter(PARAM_CHANNELS_LIST_NAME);
        $channel_list = empty($channel_list) ? 'default' : $channel_list;
        $favorites = 'favorite_channels_' . hash('crc32', $channel_list);
        if (isset($plugin_cookies->{$favorites})) {
            hd_debug_print("Upgrade $favorites");
            $ids = array_filter(explode(",", $plugin_cookies->{$favorites}));
            HD::put_data_items($favorites, $ids);
            unset($plugin_cookies->{$favorites});
        }
    }

    /**
     * @return void
     */
    public function update_log_level()
    {
        set_debug_log($this->get_bool_parameter(PARAM_ENABLE_DEBUG, false));
    }

    /**
     * @return Hashed_Array
     */
    public function get_channels_zoom()
    {
        return $this->get_parameter(PARAM_CHANNELS_ZOOM, new Hashed_Array());
    }

    /**
     * @return string
     */
    public function get_channel_zoom($channel_id)
    {
        $channels_zoom = $this->get_channels_zoom();

        if (is_null($channels_zoom)) {
            return DuneVideoZoomPresets::not_set;
        }

        $zoom = $channels_zoom->get($channel_id);
        return is_null($zoom) ? DuneVideoZoomPresets::not_set : $zoom;
    }

    /**
     * @param string $channel_id
     * @param string|null $preset
     * @return void
     */
    public function set_channel_zoom($channel_id, $preset)
    {
        $channels_zoom = $this->get_channels_zoom();
        if (!is_null($channels_zoom)) {
            if ($preset === null) {
                $channels_zoom->erase($channel_id);
            } else {
                $channels_zoom->set($channel_id, $preset);
            }
        }
        $this->set_parameter(PARAM_CHANNELS_ZOOM, $channels_zoom);
    }

    /**
     * @param string $channel_id
     * @param bool $external
     * @return void
     */
    public function set_channel_for_ext_player($channel_id, $external)
    {
        $ext_player = $this->get_channels_for_ext_player();

        if (!is_null($ext_player)){
            if ($external) {
                $ext_player->add_item($channel_id);
            } else {
                $ext_player->remove_item($channel_id);
            }
        }
        $this->set_parameter(PARAM_CHANNEL_PLAYER, $ext_player);
    }

    /**
     * @return Ordered_Array
     */
    public function get_channels_for_ext_player()
    {
        return $this->get_parameter(PARAM_CHANNEL_PLAYER, new Ordered_Array());
    }

    /**
     * @return bool
     */
    public function is_channel_for_ext_player($channel_id)
    {
        $ext_player = $this->get_channels_for_ext_player();
        return !is_null($ext_player) && $ext_player->in_order($channel_id);
    }

    /**
     * @param MediaURL $media_url
     * @param int $archive_ts
     * @throws Exception
     */
    public function tv_player_exec($media_url, $archive_ts = -1)
    {
        $url = $this->config->GenerateStreamUrl($this->tv->get_channel($media_url->channel_id), $archive_ts);

        if (!$this->is_channel_for_ext_player($media_url->channel_id)) {
            return Action_Factory::tv_play($media_url);
        }

        $url = str_replace("ts://", "", $url);
        $param_pos = strpos($url, '|||dune_params');
        $url =  $param_pos!== false ? substr($url, 0, $param_pos) : $url;
        $cmd = 'am start -d "' . $url . '" -t "video/*" -a android.intent.action.VIEW 2>&1';
        hd_debug_print("play movie in the external player: $cmd");
        /** @var array $output */
        exec($cmd, $output);
        hd_debug_print("external player exec result code" . HD::ArrayToStr($output));
        return null;
    }

    /**
     * @param array $vod_info
     * @param bool $is_external
     * @param null $post_action
     * @return array|null
     */
    public function vod_player_exec($vod_info, $is_external = false, $post_action = null)
    {
        if (!isset($vod_info[PluginVodInfo::initial_series_ndx], $vod_info[PluginVodInfo::series][$vod_info[PluginVodInfo::initial_series_ndx]])) {
            return null;
        }

        if (!$is_external) {
            return Action_Factory::vod_play($vod_info);
        }

        $series = $vod_info[PluginVodInfo::series];
        $idx = $vod_info[PluginVodInfo::initial_series_ndx];
        $url = $series[$idx][PluginVodSeriesInfo::playback_url];
        $param_pos = strpos($url, '|||dune_params');
        $url = $param_pos !== false ? substr($url, 0, $param_pos) : $url;
        $cmd = 'am start -d "' . $url . '" -t "video/*" -a android.intent.action.VIEW 2>&1';
        hd_debug_print("play movie in the external player: $cmd");
        /** @var array $output */
        exec($cmd, $output);
        hd_debug_print("external player exec result code" . HD::ArrayToStr($output));
        return $post_action;
    }

    /**
     * @param string|null $filename
     * @return string
     */
    public function get_history_path($filename = null)
    {
        $param = $this->get_parameter(PARAM_HISTORY_PATH);
        if (empty($param)) {
            $path = get_data_path(self::HISTORY_FOLDER);
        } else {
            $path = smb_tree::get_folder_info($param);
            $path = get_slash_trailed_path($path);
            if ($path === get_data_path() || $path === get_data_path(self::HISTORY_FOLDER)) {
                // reset old settings to new
                $this->remove_parameter(PARAM_HISTORY_PATH);
                $path = get_data_path(self::HISTORY_FOLDER);
            }
        }

        if ($filename !== null) {
            $path .= $filename;
        }

        return $path;
    }

    /**
     * @return Ordered_Array
     */
    public function &get_favorites()
    {
        if ($this->favorite_ids === null) {
            $channel_list = $this->get_parameter(PARAM_CHANNELS_LIST_NAME);
            $channel_list = empty($channel_list) ? 'default' : $channel_list;
            $ids = HD::get_data_items('favorite_channels_' . hash('crc32', $channel_list));

            $this->favorite_ids = new Ordered_Array(array_unique($ids));
            if (LogSeverity::$is_debug) {
                hd_debug_print("Load favorites: " . $this->favorite_ids);
            }
        }

        return $this->favorite_ids;
    }

    /**
     * @return Ordered_Array
     */
    public function &get_channels_list_favorites()
    {
        return $this->favorite_channel_list_ids;
    }

    /**
     * @return void
     */
    public function unload_favorites()
    {
        $this->favorite_ids = null;
    }

    /**
     * @return void
     */
    public function save_favorites()
    {
        $channel_list = $this->get_parameter(PARAM_CHANNELS_LIST_NAME);
        $channel_list = empty($channel_list) ? 'default' : $channel_list;
        if (LogSeverity::$is_debug) {
            hd_debug_print("Save favorites: " . $this->favorite_ids);
        }
        HD::put_data_items('favorite_channels_' . hash('crc32', $channel_list), $this->favorite_ids->get_order());
    }

    /**
     * @param User_Input_Handler $handler
     * @param string $action_id
     * @param string $caption
     * @param string $icon
     * @param $add_params array|null
     * @return array
     */
    public function create_menu_item($handler, $action_id, $caption = null, $icon = null, $add_params = null)
    {
        if ($action_id === GuiMenuItemDef::is_separator) {
            return array($action_id => true);
        }

        return User_Input_Handler_Registry::create_popup_item($handler,
            $action_id, $caption, ($icon === null) ? null : get_image_path($icon), $add_params);
    }

    /**
     * clear memory cache and entire cache folder
     *
     * @return void
     */
    public function clear_all_epg_cache()
    {
        if (isset($this->epg_manager)) {
            $this->epg_manager->get_indexer()->clear_all_epg_files();
        }
    }

    /**
     * @return void
     */
    public function run_bg_epg_indexing()
    {
        hd_debug_print(null, true);

        $sources = $this->get_all_xmltv_sources();

        foreach ($sources as $key => $value) {
            if (empty($value->url)) continue;

            hd_debug_print("Run background indexing for: ($key) $value");
            $config = array(
                'debug' => LogSeverity::$is_debug,
                'cache_dir' => $this->get_cache_dir(),
                'current_xmltv_source' => $key,
                'active_xmltv_sources' => $sources->to_array()
            );
            $config_file = get_temp_path(sprintf(self::PARSE_CONFIG, $key));
            hd_debug_print("Config: " . pretty_json_format($config), true);
            file_put_contents($config_file, pretty_json_format($config, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES));

            $cmd = get_install_path('bin/cgi_wrapper.sh') . " index_epg.php $config_file &";
            hd_debug_print("exec: $cmd", true);
            exec($cmd);
        }
    }

    /**
     * @param $channel_id
     * @return array|null
     */
    public function do_show_channel_info($channel_id)
    {
        $channel = $this->tv->get_channel($channel_id);
        if (is_null($channel)) {
            return null;
        }

        $info  = "ID: {$channel->get_id()}" . PHP_EOL;
        $info .= "Name: {$channel->get_title()}" . PHP_EOL;
        $info .= "Archive: " . var_export($channel->get_archive(), true) . " day's" . PHP_EOL;
        $info .= "Protected: " . var_export($channel->is_protected(), true) . PHP_EOL;
        $info .= "EPG IDs: " . implode(', ', $channel->get_epg_ids()) . PHP_EOL;
        if ($channel->get_timeshift_hours() !== 0 || $channel->get_timeshift_mins() !== 0) {
            $info .= sprintf("Timeshift hours: %d:%02d", $channel->get_timeshift_hours(),$channel->get_timeshift_mins()) . PHP_EOL;
        }
        $groups = array();
        foreach ($channel->get_groups() as $group) {
            $groups[] = $group->get_title();
        }
        $info .= "Categories: " . implode(', ', $groups) . PHP_EOL;
        $info .= "Icon: " . wrap_string_to_lines($channel->get_icon_url(), 70) . PHP_EOL;
        $info .= PHP_EOL;

        $live_url = '';
        try {
            $live_url = $this->config->GenerateStreamUrl($channel, -1, true);
            $info .= "Live URL: " . wrap_string_to_lines($live_url, 76) . PHP_EOL;
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
        }

        try {
            $archive_url = $this->config->GenerateStreamUrl($channel, time() - 3600, true);
            $info .= "Archive URL: " . wrap_string_to_lines($archive_url, 76) . PHP_EOL;
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
        }

        $dune_params = $this->config->UpdateDuneParams('');
        if (!empty($dune_params)) {
            $info .= "dune_params: " . substr($dune_params, strlen(HD::DUNE_PARAMS_MAGIC)) . PHP_EOL;
        }

        if (!empty($live_url) && !is_limited_apk()) {
            $descriptors = array(
                0 => array("pipe", "r"), // stdin
                1 => array("pipe", "w"), // sdout
                2 => array("pipe", "w"), // stderr
            );

            hd_debug_print("Get media info for: $live_url");
            /** @var array $pipes */
            $process = proc_open(
                get_install_path("bin/media_check.sh $live_url"),
                $descriptors,
                $pipes);

            if (is_resource($process)) {
                $output = stream_get_contents($pipes[1]);

                fclose($pipes[1]);
                proc_close($process);

                $info .= PHP_EOL;
                foreach(explode(PHP_EOL, $output) as $line) {
                    $line = trim($line);
                    if (empty($line)) continue;
                    if (strpos($line, "Output") !== false) break;
                    if (strpos($line, "Stream") !== false) {
                        $info .= preg_replace("/ \([\[].*\)| \[.*\]|, [0-9k\.]+ tb[rcn]|, q=[0-9\-]+/", "", $line) . PHP_EOL;
                    }
                }
            }
        }

        Control_Factory::add_multiline_label($defs, null, $info, 18);
        Control_Factory::add_vgap($defs, 10);

        $text = sprintf("<gap width=%s/><icon>%s</icon><gap width=10/><icon>%s</icon><text color=%s size=small>  %s</text>",
            1200,
            get_image_path('page_plus_btn.png'),
            get_image_path('page_minus_btn.png'),
            DEF_LABEL_TEXT_COLOR_SILVER,
            TR::load('scroll_page')
        );
        Control_Factory::add_smart_label($defs, '', $text);
        Control_Factory::add_vgap($defs, -80);

        Control_Factory::add_close_dialog_button($defs, TR::t('ok'), 250, true);
        Control_Factory::add_vgap($defs, 10);

        return Action_Factory::show_dialog(TR::t('channel_info_dlg'), $defs, true, 1750);
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    public function get_screen_view($name)
    {
        return isset($this->screens_views[$name]) ? $this->screens_views[$name] : array();
    }

    /**
     * @return void
     */
    public function create_screen_views()
    {
        hd_debug_print(null, true);

        $background = $this->config->plugin_info['app_background'];
        hd_debug_print("Selected background: $background", true);

        $this->screens_views = array(

            // 1x10 title list view with right side icon
            'list_1x11_small_info' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 11,
                    ViewParams::paint_icon_selection_box=> true,
                    ViewParams::paint_details => true,
                    ViewParams::paint_details_box_background => true,
                    ViewParams::paint_content_box_background => true,
                    ViewParams::paint_scrollbar => true,
                    ViewParams::paint_widget => true,
                    ViewParams::paint_help_line => true,
                    ViewParams::help_line_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_TURQUOISE,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_SMALL,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::zoom_detailed_icon => false,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_width => 60,
                    ViewItemParams::icon_height => 60,
                    ViewItemParams::icon_dx => 35,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 30,
                    ViewItemParams::item_caption_width => 1100,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            'list_1x11_normal_info' => array(
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 11,
                    ViewParams::paint_icon_selection_box=> true,
                    ViewParams::paint_details => true,
                    ViewParams::paint_details_box_background => true,
                    ViewParams::paint_content_box_background => true,
                    ViewParams::paint_scrollbar => true,
                    ViewParams::paint_widget => true,
                    ViewParams::paint_help_line => true,
                    ViewParams::help_line_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_TURQUOISE,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::zoom_detailed_icon => false,
                ),
                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_width => 60,
                    ViewItemParams::icon_height => 60,
                    ViewItemParams::icon_dx => 35,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 30,
                    ViewItemParams::item_caption_width => 1100,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),
                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'list_2x11_small_info' => array(
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 11,
                    ViewParams::paint_details => true,
                    ViewParams::paint_details_box_background => true,
                    ViewParams::paint_content_box_background => true,
                    ViewParams::paint_scrollbar => true,
                    ViewParams::paint_widget => true,
                    ViewParams::paint_help_line => true,
                    ViewParams::help_line_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_TURQUOISE,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_SMALL,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),
                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_width => 60,
                    ViewItemParams::icon_height => 60,
                    ViewItemParams::icon_dx => 35,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 74,
                    ViewItemParams::item_caption_width => 550,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),
                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'list_3x11_no_info' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 11,
                    ViewParams::paint_details => false,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_width => 60,
                    ViewItemParams::icon_height => 60,
                    ViewItemParams::icon_dx => 35,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 97,
                    ViewItemParams::item_caption_width => 600,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_5x3_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 3,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_5x3_no_caption' => array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_5x4_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 0.9,
                    ViewItemParams::icon_sel_scale_factor => 1.0,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_5x4_no_caption' => array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 0.9,
                    ViewItemParams::icon_sel_scale_factor => 1.0,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_4x3_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.2,
                    ViewItemParams::icon_sel_scale_factor => 1.4,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_4x3_no_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.2,
                    ViewItemParams::icon_sel_scale_factor => 1.4,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_3x3_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.2,
                    ViewItemParams::icon_sel_scale_factor => 1.4,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_3x3_no_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.2,
                    ViewItemParams::icon_sel_scale_factor => 1.4,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_5x2_movie_no_caption' => array(
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 2,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,

                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_SMALL,

                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => self::VOD_ICON_PATH,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => self::VOD_CHANNEL_ICON_WIDTH,
                    ViewItemParams::icon_height => self::VOD_CHANNEL_ICON_HEIGHT,
                    ViewItemParams::icon_sel_margin_top => 0,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::item_caption_width => 1100
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            'icons_7x4_no_caption' => array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 7,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH_SMALL,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT_SMALL,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 0.9,
                    ViewItemParams::icon_sel_scale_factor => 1.0,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'icons_7x4_caption' => array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 7,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH_SMALL,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT_SMALL,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 0.7,
                    ViewItemParams::icon_sel_scale_factor => 0.8,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            'list_1x10_movie_info_normal' => array(
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 10,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,

                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_SMALL,

                    ViewParams::paint_sandwich => false,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => self::VOD_ICON_PATH,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 12,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => 50,
                    ViewItemParams::icon_height => 50,
                    ViewItemParams::icon_sel_margin_top => 0,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::item_caption_width => 1100,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            'list_1x12_vod_info_normal' => array(
                PluginRegularFolderView::async_icon_loading => false,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_details => true,
                    ViewParams::background_path => $background,
                    ViewParams::background_order => 'before_all',
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                ),
                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 20,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => 50,
                    ViewItemParams::icon_height => 55,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::item_caption_width => 1100
                ),
                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }
}

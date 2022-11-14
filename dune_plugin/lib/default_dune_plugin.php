<?php
///////////////////////////////////////////////////////////////////////////

require_once 'tv/tv.php';
require_once 'mediaurl.php';
require_once 'user_input_handler_registry.php';
require_once 'action_factory.php';
require_once 'control_factory.php';
require_once 'control_factory_ext.php';
require_once 'plugin_constants.php';

class Default_Dune_Plugin implements DunePlugin
{
    const SANDWICH_BASE = 'gui_skin://special_icons/sandwich_base.aai';
    const SANDWICH_MASK = 'cut_icon://{name=sandwich_mask}';
    const SANDWICH_COVER = 'cut_icon://{name=sandwich_cover}';

    /////////////////////////////////////////////////////////////////////////////
    // views constants
    const ALL_CHANNEL_GROUP_ID = '##all_channels##';
    const ALL_CHANNEL_GROUP_CAPTION = 'Все каналы';
    const ALL_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/all.png';

    const FAV_CHANNEL_GROUP_ID = '##favorites##';
    const FAV_CHANNEL_GROUP_CAPTION = 'Избранное';
    const FAV_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/fav.png';

    const PLAYBACK_HISTORY_GROUP_ID = '##playback_history_tv_group##';

    const FAV_MOVIES_CATEGORY_CAPTION = 'Избранное';
    const FAV_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/fav_movie.png';

    const VOD_GROUP_CAPTION = 'Медиатека';
    const VOD_GROUP_ICON = "plugin_file://icons/vod.png";

    const SEARCH_MOVIES_CATEGORY_CAPTION = 'Поиск';
    const SEARCH_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/search_movie.png';

    const FILTER_MOVIES_CATEGORY_CAPTION = 'Фильтр';
    const FILTER_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/filter_movie.png';

    const HISTORY_MOVIES_CATEGORY_CAPTION = 'История просмотра';
    const HISTORY_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/history_movie.png';

    /////////////////////////////////////////////////////////////////////////////
    // views variables
    const TV_SANDWICH_WIDTH = 245;
    const TV_SANDWICH_HEIGHT = 140;

    const VOD_SANDWICH_WIDTH = 190;
    const VOD_SANDWICH_HEIGHT = 290;

    const VOD_CHANNEL_ICON_WIDTH = 190;
    const VOD_CHANNEL_ICON_HEIGHT = 290;

    const DEFAULT_CHANNEL_ICON_PATH = 'plugin_file://icons/channel_unset.png';
    const DEFAULT_MOV_ICON_PATH = 'plugin_file://icons/mov_unset.png';
    const VOD_ICON_PATH = 'gui_skin://small_icons/movie.aai';

    /**
     * @var bool
     */
    public $new_ui_support;

    /**
     * @var bool
     */
    public $history_support;

    // info
    public $plugin_info;

    /**
     * set base plugin info from dune_plugin.xml
     * @throws Exception
     */
    public function plugin_setup()
    {
        $this->plugin_info = get_plugin_manifest_info();

        $plugin_config_class = $this->plugin_info['app_class_name'];

        if (!class_exists($plugin_config_class)) {
            hd_print("Unknown plugin: $plugin_config_class");
            throw new Exception("Unknown plugin type: $plugin_config_class");
        }

        if (!is_subclass_of($plugin_config_class, 'dynamic_config')) {
            hd_print("plugin: $plugin_config_class not a subclass of 'dynamic_config'");
            throw new Exception("Wrong subclass: $plugin_config_class");
        }

        hd_print("Instantiate class: $plugin_config_class");
        $this->config = new $plugin_config_class;
        $this->config->init_defaults();
        $this->config->load_config();
        $this->config->load_embedded_account();

        print_sysinfo();

        hd_print("----------------------------------------------------");
        hd_print("Plugin ID:        " . $this->plugin_info['app_short_name']);
        hd_print("Plugin name:      " . $this->plugin_info['app_caption']);
        hd_print("Plugin version:   " . $this->plugin_info['app_version'] . '.' . $this->plugin_info['app_version_idx']);
        hd_print("Plugin date:      " . $this->plugin_info['app_release_date']);
        hd_print("Account type:     " . $this->config->get_feature(Plugin_Constants::ACCESS_TYPE));
        hd_print("TV fav:           " . ($this->config->get_feature(Plugin_Constants::TV_FAVORITES_SUPPORTED) ? "yes" : "no"));
        hd_print("VOD page:         " . ($this->config->get_feature(Plugin_Constants::VOD_SUPPORTED) ? "yes" : "no"));
        hd_print("LocalTime         " . format_datetime('Y-m-d H:i', time()));
        hd_print("TimeZone          " . getTimeZone());
        hd_print("Daylight          " . date('I') ? 'yes' : 'no');
        hd_print("Icon              " . $this->plugin_info['app_logo']);
        hd_print("Background        " . $this->plugin_info['app_background']);
        hd_print("Channels path     " . $this->plugin_info['app_channels_url_path']);
        hd_print("----------------------------------------------------");
    }

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
     * @var Starnet_Main_Screen
     */
    public $main_screen;

    /**
     * @var Starnet_Tv_Channel_List_Screen
     */
    public $tv_channels_screen;

    /**
     * @var Starnet_Setup_Screen
     */
    public $setup_screen;

    /**
     * @var Starnet_Folder_Screen
     */
    public $folder_screen;

    /**
     * @var Starnet_Tv_Favorites_Screen
     */
    public $tv_favorites_screen;

    /**
     * @var Starnet_Vod_Search_Screen
     */
    public $vod_search_screen;

    /**
     * @var Starnet_Vod_Favorites_Screen
     */
    public $vod_favorites_screen;

    /**
     * @var Starnet_Vod_Category_List_Screen
     */
    public $vod_category_list_Screen;

    /**
     * @var Starnet_Vod_List_Screen
     */
    public $vod_list_screen;

    /**
     * @var Starnet_Vod_Movie_Screen
     */
    public $vod_movie_screen;

    /**
     * @var Starnet_Vod_Seasons_List_Screen
     */
    public $vod_season_List_Screen;

    /**
     * @var Starnet_Vod_Series_List_Screen
     */
    public $vod_series_list_screen;

    /**
     * @var Starnet_Vod_History_Screen
     */
    public $vod_history_screen;

    /**
     * @var Starnet_Vod_Filter_Screen
     */
    public $vod_filter_screen;

    /**
     * @var array|Screen[]
     */
    private $screens;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct()
    {
        $this->screens = array();
        $this->new_ui_support = HD::rows_api_support();
        $this->history_support = $this->new_ui_support && class_exists('Playback_Points');
    }

    /**
     * @param $object
     */
    public function create_screen(&$object)
    {
        if (!is_null($object) && method_exists($object, 'get_id')) {
            hd_print('create_screen: ' . get_class($object));
            $this->add_screen($object);
            User_Input_Handler_Registry::get_instance()->register_handler($object);
        } else {
            hd_print(get_class($object) . ': Screen class is illegal. get_id method not defined!');
        }
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
            hd_print("Error: screen (id: " . $scr->get_id() . ") already registered.");
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
            // hd_print("get_screen_by_id: '$screen_id'");
            return $this->screens[$screen_id];
        }

        hd_print("Error: no screen with id '$screen_id' found.");
        HD::print_backtrace();
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
     * @throws Exception
     */
    public function get_tv_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_info: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        $decoded_media_url = MediaURL::decode($media_url);

        return $this->tv->get_tv_info($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return string
     * @throws Exception
     */
    public function get_tv_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_stream_url: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param int $archive_tm_sec
     * @param string $protect_code
     * @param $plugin_cookies
     * @return string
     * @throws Exception
     */
    public function get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_playback_url: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param int $day_start_tm_sec
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_day_epg($channel_id, $day_start_tm_sec, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_day_epg: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_day_epg($channel_id, $day_start_tm_sec, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $op_type
     * @param string $channel_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function change_tv_favorites($op_type, $channel_id, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('change_tv_favorites: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->change_tv_favorites($op_type, $channel_id, $plugin_cookies);
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
     * @throws Exception
     */
    public function get_vod_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print('get_vod_info: VOD is not supported');
            HD::print_backtrace();
            throw new Exception('VOD is not supported');
        }

        //hd_print("Default_Dune_Plugin::get_vod_info: MediaUrl: $media_url_str");
        $mu = MediaURL::decode($media_url);

        return $this->vod->get_vod_info($mu, $plugin_cookies);
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
            hd_print('get_vod_stream_url: VOD is not supported');
            return '';
        }

        return $this->vod->get_vod_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Misc.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //if ($user_input->control_id !== 'timer' && $user_input->control_id !== 'plugin_rows_info_update') {
        //    hd_print('Default_Dune_Plugin: handle_user_input:');
        //    foreach ($user_input as $key => $value) hd_print("  $key => $value");
        //}

        return User_Input_Handler_Registry::get_instance()->handle_user_input($user_input, $plugin_cookies);
    }

    /**
     * @param string $image
     * @return string
     */
    public function get_image_path($image = null)
    {
        return get_install_path("/img/" . ($image === null ?: $image));
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    /**
     * @return array[]
     */
    public function GET_TV_GROUP_LIST_FOLDER_VIEWS()
    {
        return array(

            // small no caption
            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // small with caption
            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 3,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => false,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.2,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // large, no caption
            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => false,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // 3x10 list view
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 60 : 84,
                    ViewItemParams::icon_height => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 60 : 48,
                    ViewItemParams::item_caption_width => 485,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::item_caption_dx => 50,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 1x10 list view with info
            array
            (
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 10,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,

                    ViewParams::paint_sandwich => false,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => static::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => static::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    //ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => self::VOD_ICON_PATH,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 14,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 50 : 52,
                    ViewItemParams::icon_height => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 50 : 34,
                    ViewItemParams::icon_sel_margin_top => 0,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::item_caption_width => 1100,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_LARGE,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),
        );
    }

    /**
     * @return array[]
     */
    public function GET_TV_CHANNEL_LIST_FOLDER_VIEWS()
    {
        return array(
            // 4x3 with title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 3x3 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 4x4 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 5x4 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::TV_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 2x10 title list view with right side icon
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 60 : 84,
                    ViewItemParams::icon_height => $this->config->get_feature(Plugin_Constants::SQUARE_ICONS) ? 60 : 48,
                    ViewItemParams::item_caption_width => 485,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),

            // 1x10 title list view with right side icon
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 10,
                    ViewParams::paint_icon_selection_box=> true,
                    ViewParams::paint_details => true,
                    ViewParams::paint_details_box_background => true,
                    ViewParams::paint_content_box_background => true,
                    ViewParams::paint_scrollbar => true,
                    ViewParams::paint_widget => true,
                    ViewParams::paint_help_line => true,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_SMALL,
                    ViewParams::background_path=> $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::item_detailed_info_text_color => 11,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::optimize_full_screen_background => true,
                    ViewParams::zoom_detailed_icon => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_width => 50,
                    ViewItemParams::icon_height => 50,
                    ViewItemParams::icon_dx => 26,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                    ViewItemParams::item_caption_width => 1060,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),
        );
    }

    /**
     * @return array[]
     */
    public function GET_VOD_MOVIE_LIST_FOLDER_VIEWS()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 2,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,

                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => self::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => self::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
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

            array
            (
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 10,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,

                    ViewParams::paint_sandwich => false,
                    ViewParams::sandwich_base => self::SANDWICH_BASE,
                    ViewParams::sandwich_mask => self::SANDWICH_MASK,
                    ViewParams::sandwich_cover => self::SANDWICH_COVER,
                    ViewParams::sandwich_width => static::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => static::VOD_SANDWICH_HEIGHT,
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
                    ViewItemParams::item_caption_font_size => FONT_SIZE_LARGE,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),
        );
    }

    /**
     * @return array[]
     */
    public function GET_VOD_CATEGORY_LIST_FOLDER_VIEWS()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => false,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_details => true,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
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

    /**
     * @return array[]
     */
    public function GET_TEXT_ONE_COL_VIEWS()
    {
        return array(
            array(
                PluginRegularFolderView::async_icon_loading => false,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_details => true,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                ),
                PluginRegularFolderView::base_view_item_params =>
                    array
                    (
                        ViewItemParams::icon_path => 'missing://',
                        ViewItemParams::item_layout => HALIGN_LEFT,
                        ViewItemParams::icon_valign => VALIGN_CENTER,
                        ViewItemParams::icon_dx => 20,
                        ViewItemParams::icon_dy => -5,
                        ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                        ViewItemParams::item_caption_width => 1550
                    ),
                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }

    /**
     * @return array
     */
    public function GET_FOLDER_VIEWS()
    {
        if (defined('ViewParams::details_box_width')) {
            $view[] = array(
                PluginRegularFolderView::async_icon_loading => false,
                PluginRegularFolderView::view_params => array(
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_content_box_background => false,
                    ViewParams::paint_icon_selection_box => true,
                    ViewParams::paint_details_box_background => false,
                    ViewParams::icon_selection_box_width => 770,
                    ViewParams::paint_path_box_background => false,
                    ViewParams::paint_widget_background => false,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::details_box_width => 900,
                    ViewParams::paint_scrollbar => false,
                    ViewParams::content_box_padding_right => 500,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                ),
                PluginRegularFolderView::base_view_item_params => array(
                    ViewItemParams::item_layout => 0,
                    ViewItemParams::icon_width => 30,
                    ViewItemParams::icon_height => 50,
                    ViewItemParams::item_caption_dx => 55,
                    ViewItemParams::icon_dx => 5,
                    ViewItemParams::icon_sel_scale_factor => 1.01,
                    ViewItemParams::icon_keep_aspect_ratio => true,
                    ViewItemParams::icon_sel_dx => 6,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_valign => 1,
                ),
                PluginRegularFolderView::not_loaded_view_item_params => array(),
            );
        }

        $view[] = array(
            PluginRegularFolderView::view_params => array(
                ViewParams::num_cols => 1,
                ViewParams::num_rows => 10,
                ViewParams::paint_details => true,
                ViewParams::paint_item_info_in_details => true,
                ViewParams::detailed_icon_scale_factor => 0.5,
                ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                ViewParams::item_detailed_info_auto_line_break => true
            ),
            PluginRegularFolderView::base_view_item_params => array(
                ViewItemParams::item_paint_icon => true,
                ViewItemParams::icon_sel_scale_factor => 1.2,
                ViewItemParams::item_layout => HALIGN_LEFT,
                ViewItemParams::icon_valign => VALIGN_CENTER,
                ViewItemParams::icon_dx => 10,
                ViewItemParams::icon_dy => -5,
                ViewItemParams::icon_width => 50,
                ViewItemParams::icon_height => 50,
                ViewItemParams::icon_sel_margin_top => 0,
                ViewItemParams::item_paint_caption => true,
                ViewItemParams::item_caption_width => 1100,
                ViewItemParams::item_detailed_icon_path => 'missing://'
            ),
            PluginRegularFolderView::not_loaded_view_item_params => array(),
            PluginRegularFolderView::async_icon_loading => false,
            PluginRegularFolderView::timer => Action_Factory::timer(5000),
        );
        return $view;
    }

    /**
     * @return array[]
     */
    public function GET_VOD_SERIES_FOLDER_VIEW()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_path_box => true,
                    ViewParams::paint_details => true,
                    ViewParams::paint_item_info_in_details => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => DEF_LABEL_TEXT_COLOR_GREEN,
                    ViewParams::item_detailed_info_text_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewParams::background_path => $this->plugin_info['app_background'],
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 60,
                    ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai',
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH,
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ),
        );
    }
}

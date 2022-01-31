<?php
require_once 'lib/epg_manager.php';
require_once 'lib/tv/channel.php';

const ACCOUNT_TYPE = 'account';
const TV_FAVORITES_SUPPORTED = 'tv_fav';
const VOD_MOVIE_PAGE_SUPPORTED = 'vod_support';
const VOD_FAVORITES_SUPPORTED = 'vod_fav';
const VOD_PORTAL_SUPPORTED = 'vod_portal';
const TS_OPTIONS = 'ts_options';
const BALANCE_SUPPORTED = 'balance_support';
const DEVICE_OPTIONS = 'device_options';
const SERVER_SUPPORTED = 'server_support';
const QUALITY_SUPPORTED = 'quality_support';
const M3U_STREAM_URL_PATTERN = 'm3u8_pattern';
const MEDIA_URL_TEMPLATE_HLS = 'hls_url';
const VOD_LAZY_LOAD = 'vod_lazy';
const EXTINF_VOD_PATTERN = 'vod_pattern';
const SQUARE_ICONS = 'square_icons';

abstract class DefaultConfig
{
    // features constants

    // info
    public static $PLUGIN_SHOW_NAME = 'StarNet';
    public static $PLUGIN_SHORT_NAME = 'starnet';
    public static $PLUGIN_VERSION = '0.0.0';
    public static $PLUGIN_DATE = '04.01.1972';

    protected static $FEATURES = array();
    protected static $EPG_PARSER_PARAMS = array();

    // page counter for some plugins
    protected static $pages = array();
    protected static $is_entered = false;
    protected static $movie_counter = array();
    protected static $filters = array();

    /////////////////////////////////////////////////////////////////////////////
    // views constants
    const ALL_CHANNEL_GROUP_CAPTION = 'Все каналы';
    const ALL_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/all.png';

    const FAV_CHANNEL_GROUP_ID = '__favorites';
    const FAV_CHANNEL_GROUP_CAPTION = 'Избранное';
    const FAV_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/fav.png';

    const FAV_MOVIES_CATEGORY_CAPTION = 'Избранное';
    const FAV_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/fav_movie.png';

    const VOD_GROUP_CAPTION = 'Медиатека';
    const VOD_GROUP_ICON = "plugin_file://icons/vod.png";
    const VOD_ICON_PATH = 'gui_skin://small_icons/movie.aai';

    const SEARCH_MOVIES_CATEGORY_CAPTION = 'Поиск';
    const SEARCH_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/search_movie.png';

    const FILTER_MOVIES_CATEGORY_CAPTION = 'Фильтр';
    const FILTER_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/filter_movie.png';

    const DEFAULT_CHANNEL_ICON_PATH = 'plugin_file://icons/channel_unset.png';
    const DEFAULT_MOV_ICON_PATH = 'plugin_file://icons/mov_unset.png';

    const SANDWICH_BASE = 'gui_skin://special_icons/sandwich_base.aai';
    const SANDWICH_MASK = 'cut_icon://{name=sandwich_mask}';
    const SANDWICH_COVER = 'cut_icon://{name=sandwich_cover}';

    /////////////////////////////////////////////////////////////////////////////
    // views variables
    const TV_SANDWICH_WIDTH = 245;
    const TV_SANDWICH_HEIGHT = 140;

    const VOD_SANDWICH_WIDTH = 190;
    const VOD_SANDWICH_HEIGHT = 290;

    const VOD_CHANNEL_ICON_WIDTH = 190;
    const VOD_CHANNEL_ICON_HEIGHT = 290;

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $xml = HD::parse_xml_file(HD::get_install_path('dune_plugin.xml'));

        static::$PLUGIN_SHOW_NAME = $xml->caption;
        static::$PLUGIN_SHORT_NAME = $xml->short_name;
        static::$PLUGIN_VERSION = $xml->version;

        static::$FEATURES[ACCOUNT_TYPE] = 'UNKNOWN';
        static::$FEATURES[TV_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = false;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = false;
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS', 'mpeg' => 'MPEG-TS');
        static::$FEATURES[DEVICE_OPTIONS] = array();
        static::$FEATURES[BALANCE_SUPPORTED] = false;
        static::$FEATURES[SERVER_SUPPORTED] = false;
        static::$FEATURES[QUALITY_SUPPORTED] = false;
        static::$FEATURES[VOD_LAZY_LOAD] = false;
        static::$FEATURES[EXTINF_VOD_PATTERN] = '';
        static::$FEATURES[SQUARE_ICONS] = false;

        static::$EPG_PARSER_PARAMS['first']['parser'] = 'json';
        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'epg_data';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'time';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'time_to';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'name';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'descr';
        static::$EPG_PARSER_PARAMS['first']['date_format'] = 'Y-m-d';

        static::$EPG_PARSER_PARAMS['second']['parser'] = 'json';
        static::$EPG_PARSER_PARAMS['second']['epg_root'] = 'epg_data';
        static::$EPG_PARSER_PARAMS['second']['start'] = 'time';
        static::$EPG_PARSER_PARAMS['second']['end'] = 'time_to';
        static::$EPG_PARSER_PARAMS['second']['title'] = 'name';
        static::$EPG_PARSER_PARAMS['second']['description'] = 'descr';
        static::$EPG_PARSER_PARAMS['second']['date_format'] = 'Y-m-d';
    }

    public static function get_account_type()
    {
        return self::$FEATURES[ACCOUNT_TYPE];
    }

    public static function get_tv_fav_support()
    {
        return self::$FEATURES[TV_FAVORITES_SUPPORTED];
    }

    public static function get_vod_fav_support()
    {
        return self::$FEATURES[VOD_FAVORITES_SUPPORTED];
    }

    public static function get_vod_support()
    {
        return self::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED];
    }

    public static function get_vod_portal_support()
    {
        return self::$FEATURES[VOD_PORTAL_SUPPORTED];
    }

    public static function get_format_opts()
    {
        return self::$FEATURES[TS_OPTIONS];
    }

    public static function get_balance_support()
    {
        return self::$FEATURES[BALANCE_SUPPORTED];
    }

    public static function get_device_support()
    {
        return !empty(self::$FEATURES[DEVICE_OPTIONS]);
    }

    public static function get_device_opts()
    {
        return self::$FEATURES[DEVICE_OPTIONS];
    }

    public static function get_device($plugin_cookies)
    {
        return null;
    }

    public static function set_device($plugin_cookies)
    {
    }

    public static function get_quality_support()
    {
        return self::$FEATURES[QUALITY_SUPPORTED];
    }

    public static function get_quality_opts($plugin_cookies)
    {
        return array();
    }

    public static function get_quality($plugin_cookies)
    {
        return null;
    }

    public static function set_quality($plugin_cookies)
    {
    }

    public static function get_server_support()
    {
        return self::$FEATURES[SERVER_SUPPORTED];
    }

    public static function get_server_opts($plugin_cookies)
    {
        return array();
    }

    public static function get_server($plugin_cookies)
    {
        return null;
    }

    public static function set_server($plugin_cookies)
    {
        return null;
    }

    public static function get_channel_list()
    {
        return sprintf('%s_channel_list.xml', self::$PLUGIN_SHORT_NAME);
    }

    public static function sort_channels_cb($a, $b)
    {
        // Sort by channel numbers.
        return strnatcasecmp($a->get_number(), $b->get_number());
    }

    public static function get_epg_params()
    {
        return static::$EPG_PARSER_PARAMS;
    }

    public static function try_reset_pages()
    {
        if (static::$is_entered) {
            static::$is_entered = false;
            static::$pages = array();
        }
    }

    public static function reset_movie_counter()
    {
        static::$is_entered = true;
        static::$movie_counter = array();
    }

    public static function get_movie_counter($key)
    {
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        return static::$movie_counter[$key];
    }

    public static function add_movie_counter($key, $val)
    {
        // entire list available, counter is final
        static::$movie_counter[$key] = $val;
    }

    public static function get_next_page($idx, $increment = 1)
    {
        if (!array_key_exists($idx, static::$pages)) {
            static::$pages[$idx] = 0;
        }

        static::$pages[$idx] += $increment;

        return static::$pages[$idx];
    }

    public static function is_lazy_load_vod()
	{
        return static::$FEATURES[VOD_LAZY_LOAD];
    }

    public static function get_filter($name)
    {
        return isset(static::$filters[$name]) ? static::$filters[$name] : null;
    }

    public static function set_filters($filters)
    {
        static::$filters = $filters;
    }

    public static function GET_BG_PICTURE()
    {
        return sprintf('plugin_file://icons/bg_%s.jpg', self::$PLUGIN_SHORT_NAME);
    }

    public static function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        return false;
    }

    public static function CompileSaveFilterItem($user_input)
    {
        return null;
    }

    public static function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        ControlFactory::add_label($defs, 'Баланс:', 'Информация о балансе не поддерживается');
    }

    /**
     * Update url macros {DOMAIN} and {TOKEN} by values from channel ext_params
     * Make url ts wrapped
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();
        $ext_params = $channel->get_ext_params();
        if (!isset($ext_params['subdomain'])) {
            hd_print("TransformStreamUrl: parameter 'subdomain' for {$channel->get_channel_id()} not defined!");
        } else {
            $url = str_replace('{DOMAIN}', $ext_params['subdomain'], $url);
        }

        if (!isset($ext_params['token'])) {
            hd_print("TransformStreamUrl: parameter 'token' for {$channel->get_channel_id()} not defined!");
        } else {
            $url = str_replace('{TOKEN}', $ext_params['token'], $url);
        }

        return $url;
    }

    /**
     * Update url by channel ID (for correct hash calculation of url)
     * @param $channel_id
     * @param $ext_params
     * @return string
     */
    public static function UpdateStreamUrlID($channel_id, $ext_params)
    {
        return str_replace('{ID}', $channel_id, static::$FEATURES[MEDIA_URL_TEMPLATE_HLS]);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account " . static::$PLUGIN_SHOW_NAME);
        $m3u_lines = self::FetchTvM3U($plugin_cookies, $force);
        foreach ($m3u_lines as $line) {
            if (preg_match(static::$FEATURES[M3U_STREAM_URL_PATTERN], $line, $matches)) {
                $account_data = $matches;
                return true;
            }
        }

        return false;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $m3u_lines = self::FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (preg_match(self::$FEATURES[M3U_STREAM_URL_PATTERN], $line, $matches)) {
                $pl_entries[$matches['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            unlink(self::GET_TMP_STORAGE_PATH());
            throw new DuneException(
                'Empty provider playlist', 0,
                ActionFactory::show_error(
                    true,
                    'Ошибка скачивания плейлиста',
                    array(
                        'Пустой плейлист провайдера!',
                        'Проверьте подписку или подключение к Интернет.')));
        }

        return $pl_entries;
    }

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match(static::$FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

            $search = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new ShortMovie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    public static function getFilterList($params, $plugin_cookies)
    {
        //hd_print("getFilterList: $params");
        return array();
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();
        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match(static::$FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            $logo = $matches['logo'];
            $caption = $matches['title'];
            if(empty($category)) {
                $category = 'Без категории';
            }

            $arr = explode("_", $query_id);
            $category_id = ($arr === false) ? $query_id : $arr[0];
            if ($category_id === $category) {
                $movies[] = new ShortMovie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies read: " . count($movies));
        return $movies;
    }

    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        return null;
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        return null;
    }

    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $now_ts = time();
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    protected static function UpdateMpegTsBuffering($url, $plugin_cookies)
    {
        if (self::get_format($plugin_cookies) === 'mpeg') {
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        return HD::make_ts($url);
    }

    protected static function get_format($plugin_cookies)
    {
        // hd_print("Stream type: " . isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls');
        return isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
    }

    protected static function FetchTvM3U($plugin_cookies, $force = false)
    {
        $tmp_file = self::GET_TMP_STORAGE_PATH();
        if ($force !== false || !file_exists($tmp_file)) {
            try {
                $url = static::GetPlaylistUrl('tv1', $plugin_cookies);
                //hd_print("tv1 m3u8 playlist: " . $url);
                if (empty($url)) {
                    throw new Exception('Tv1 playlist not defined');
                }
                file_put_contents($tmp_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                try {
                    $url = static::GetPlaylistUrl('tv2', $plugin_cookies);
                    //hd_print("tv2 m3u8 playlist: " . $url);
                    if (empty($url)) {
                        hd_print("Tv2 playlist not defined");
                        return array();
                    }

                    file_put_contents($tmp_file, HD::http_get_document($url));
                } catch (Exception $ex) {
                    hd_print("Unable to load secondary tv playlist: " . $ex->getMessage());
                    return array();
                }
            }
        }

        return file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    }

    public static function FetchVodM3U($plugin_cookies, $force = false)
    {
        $m3u_file = self::GET_VOD_TMP_STORAGE_PATH();

        if ($force !== false || !file_exists($m3u_file)) {
            try {
                $url = static::GetPlaylistUrl('movie', $plugin_cookies);
                if (empty($url)) {
                    throw new Exception('Vod playlist not defined');
                }

                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print("Unable to load movie playlist: " . $ex->getMessage());
                return array();
            }
        }

        return file($m3u_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (!preg_match(static::$FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new StarnetVodCategory($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        return '';
    }

    /**
     * @return string
     */
    protected static function GET_VOD_TMP_STORAGE_PATH()
    {
        return self::GET_TMP_STORAGE_PATH('playlist_vod.m3u8');
    }

    /**
     * @return string
     */
    protected static function GET_TMP_STORAGE_PATH($name = null)
    {
        if (is_null($name)) {
            $name = 'playlist_tv.m3u8';
        }

        return sprintf('/tmp/%s_%s', self::$PLUGIN_SHORT_NAME, $name);
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    public static function GET_TV_GROUP_LIST_FOLDER_VIEWS()
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
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
        );
    }

    public static function GET_TV_CHANNEL_LIST_FOLDER_VIEWS()
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
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // 3x3 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // 4x4 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // 5x4 without title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            // 2x10 list view with title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => static::$FEATURES[SQUARE_ICONS] ? 60 : 84,
                    ViewItemParams::icon_height => static::$FEATURES[SQUARE_ICONS] ? 60 : 48,
                    ViewItemParams::item_caption_width => 485,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::icon_path => self::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }

    public static function GET_VOD_MOVIE_LIST_FOLDER_VIEWS()
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => self::VOD_CHANNEL_ICON_WIDTH,
                    ViewItemParams::icon_height => self::VOD_CHANNEL_ICON_HEIGHT,
                    ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH
                ),
                array
                (
                    PluginRegularFolderView::async_icon_loading => true,

                    PluginRegularFolderView::view_params => array
                    (
                        ViewParams::num_cols => 1,
                        ViewParams::num_rows => 3,
                        ViewParams::paint_details => true,
                        ViewParams::paint_item_info_in_details => true,
                        ViewParams::item_detailed_info_auto_line_break => true,
                        ViewParams::item_detailed_info_title_color => 10,
                        ViewParams::item_detailed_info_text_color => 15,
                        ViewParams::background_path => self::GET_BG_PICTURE(),
                        ViewParams::background_order => 0,
                        ViewParams::background_height => 1080,
                        ViewParams::background_width => 1920,
                        ViewParams::optimize_full_screen_background => true,

                        ViewParams::paint_sandwich => false,
                        ViewParams::sandwich_base => self::SANDWICH_BASE,
                        ViewParams::sandwich_mask => self::SANDWICH_MASK,
                        ViewParams::sandwich_cover => self::SANDWICH_COVER,
                        ViewParams::sandwich_width => static::VOD_SANDWICH_WIDTH,
                        ViewParams::sandwich_height => static::VOD_SANDWICH_HEIGHT,
                        ViewParams::sandwich_icon_upscale_enabled => true,
                        ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ),

                    PluginRegularFolderView::not_loaded_view_item_params => array
                    (
                        ViewItemParams::item_paint_icon => true,
                        ViewItemParams::icon_width => self::VOD_CHANNEL_ICON_WIDTH,
                        ViewItemParams::icon_height => self::VOD_CHANNEL_ICON_HEIGHT,
                        ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH
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
                        ViewItemParams::item_paint_caption => true,
                        ViewItemParams::item_caption_width => 950
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
                        ViewParams::item_detailed_info_title_color => 10,
                        ViewParams::item_detailed_info_text_color => 15,
                        ViewParams::background_path => self::GET_BG_PICTURE(),
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

                    PluginRegularFolderView::not_loaded_view_item_params => array
                    (
                        ViewItemParams::item_paint_icon => true,
                        ViewItemParams::icon_width => 50,
                        ViewItemParams::icon_height => 50,
                        ViewItemParams::icon_path => self::DEFAULT_MOV_ICON_PATH
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
                        ViewItemParams::icon_width => 50,
                        ViewItemParams::icon_height => 50,
                        ViewItemParams::icon_sel_margin_top => 0,
                        ViewItemParams::item_paint_caption => true,
                        ViewItemParams::item_caption_width => 1100
                    ),
                ),
            )
        );
    }

    public static function GET_VOD_CATEGORY_LIST_FOLDER_VIEWS()
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
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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

    public static function GET_TEXT_ONE_COL_VIEWS()
    {
        return array(
            array(
                PluginRegularFolderView::async_icon_loading => false,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_details => true,
                    ViewParams::background_path => self::GET_BG_PICTURE(),
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
}

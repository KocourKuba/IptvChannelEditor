<?php
require_once 'lib/epg_manager.php';
require_once 'lib/tv/channel.php';

const ACCOUNT_TYPE = 'account';
const TV_FAVORITES_SUPPORTED = 'tv_fav';
const VOD_MOVIE_PAGE_SUPPORTED = 'vod_support';
const VOD_FAVORITES_SUPPORTED = 'vod_fav';
const VOD_PORTAL_SUPPORTED = 'vod_portal';
const VOD_LAZY_LOAD = 'vod_lazy';
const TS_OPTIONS = 'ts_options';
const BALANCE_SUPPORTED = 'balance_support';
const DEVICE_OPTIONS = 'device_options';
const SERVER_SUPPORTED = 'server_support';
const QUALITY_SUPPORTED = 'quality_support';
const M3U_STREAM_URL_PATTERN = 'm3u8_pattern';
const MEDIA_URL_TEMPLATE_HLS = 'hls_url';
const MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'hls_archive_url';
const MEDIA_URL_TEMPLATE_MPEG = 'mpeg_url';
const MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'mpeg_archive_url';
const EXTINF_VOD_PATTERN = 'vod_pattern';
const SQUARE_ICONS = 'square_icons';
const SECONDARY_EPG = 'secondary_epg';

abstract class Default_Config
{
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

    const HISTORY_MOVIES_CATEGORY_CAPTION = 'История просмотра';
    const HISTORY_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/history_movie.png';

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

    // features constants

    // info
    public $PLUGIN_SHOW_NAME = 'StarNet';
    public $PLUGIN_SHORT_NAME = 'starnet';
    public $PLUGIN_VERSION = '0.0.0';
    public $PLUGIN_DATE = '04.01.1972';
    public $PLUGIN_CAPTION = '';
    public $PLUGIN_ICON = '';
    public $PLUGIN_BACKGROUND = '';
    public $PLUGIN_CHANNELS_URL_PATH = '';

    protected $FEATURES = array();
    protected $EPG_PARSER_PARAMS = array();

    // page counter for some plugins
    protected $pages = array();
    protected $is_entered = false;
    protected $movie_counter = array();
    protected $filters = array();

    protected $embedded_account;

    /**
     * @throws Exception
     */
    public function __construct()
    {
        $xml = HD::parse_xml_file(get_install_path('dune_plugin.xml'));

        $acc_file = get_install_path('account.dat');
        if (file_exists($acc_file)) {
            $data = file_get_contents($acc_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            if ($data !== false) {
                hd_print("account data: $data");
                $account = json_decode(base64_decode(substr($data, 5)));
                if ($account !== false) {
                    $this->embedded_account = $account;
                    //foreach ($this->embedded_account as $key => $item) {
                    //    hd_print("Embedded info: $key => $item");
                    //}
                }
            }
        }

        $this->PLUGIN_SHOW_NAME = $xml->caption;
        $this->PLUGIN_SHORT_NAME = $xml->short_name;
        $this->PLUGIN_ICON = $xml->icon_url;
        $this->PLUGIN_BACKGROUND = $xml->background;
        $this->PLUGIN_VERSION = $xml->version . '.' . $xml->version_index;
        $this->PLUGIN_DATE = $xml->release_date;
        $this->PLUGIN_CHANNELS_URL_PATH = $xml->channels_url_path;

        $this->FEATURES[ACCOUNT_TYPE] = 'UNKNOWN';
        $this->FEATURES[SECONDARY_EPG] = false;
        $this->FEATURES[TV_FAVORITES_SUPPORTED] = true;
        $this->FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = false;
        $this->FEATURES[VOD_FAVORITES_SUPPORTED] = false;
        $this->FEATURES[VOD_PORTAL_SUPPORTED] = false;
        $this->FEATURES[VOD_LAZY_LOAD] = false;
        $this->FEATURES[TS_OPTIONS] = array('hls' => 'HLS', 'mpeg' => 'MPEG-TS');
        $this->FEATURES[DEVICE_OPTIONS] = array();
        $this->FEATURES[BALANCE_SUPPORTED] = false;
        $this->FEATURES[SERVER_SUPPORTED] = false;
        $this->FEATURES[QUALITY_SUPPORTED] = false;
        $this->FEATURES[EXTINF_VOD_PATTERN] = '';
        $this->FEATURES[SQUARE_ICONS] = false;

        $default_parser = array();
        $default_parser['epg_parser'] = 'json';
        $default_parser['epg_url'] = '';
        $default_parser['epg_root'] = 'epg_data';
        $default_parser['epg_start'] = 'time';
        $default_parser['epg_end'] = 'time_to';
        $default_parser['epg_title'] = 'name';
        $default_parser['epg_desc'] = 'descr';
        $default_parser['epg_date_format'] = 'Y-m-d';
        $default_parser['epg_use_duration'] = false;
        $default_parser['epg_use_hash'] = false;
        $default_parser['epg_use_mapper'] = false;
        $default_parser['epg_mapper_url'] = '';
        $default_parser['epg_time_format'] = '';
        $default_parser['epg_timezone'] = 'UTC';
        $default_parser['epg_id_mapper'] = array();

        $this->set_epg_params('first', $default_parser);
        $this->set_epg_params('second', $default_parser);
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_feature($type)
    {
        return $this->FEATURES[$type];
    }

    /**
     * @param string $type
     * @param mixed $val
     */
    public function set_feature($type, $val)
    {
        $this->FEATURES[$type] = $val;
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_device($plugin_cookies)
    {
        return null;
    }

    public function set_device($device, $plugin_cookies)
    {
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_quality_opts($plugin_cookies)
    {
        return array();
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_quality($plugin_cookies)
    {
        return null;
    }

    public function set_quality($quality, $plugin_cookies)
    {
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_server_opts($plugin_cookies)
    {
        return array();
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_server($plugin_cookies)
    {
        return null;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server($server, $plugin_cookies)
    {
    }

    public function get_login($plugin_cookies)
    {
        return isset($this->embedded_account->login) ? $this->embedded_account->login : $plugin_cookies->login;
    }

    public function get_password($plugin_cookies)
    {
        return isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;
    }

    /**
     * @param $plugin_cookies
     * @param string &$used_list
     * @return array $all_channels
     */
    public function get_channel_list($plugin_cookies, &$used_list)
    {
        $used_list = isset($plugin_cookies->channels_list) ? (string)$plugin_cookies->channels_list : sprintf('%s_channel_list.xml', $this->PLUGIN_SHORT_NAME);
        hd_print("Default channels list name: $used_list");

        $channels_source = isset($plugin_cookies->channels_source) ? (int)$plugin_cookies->channels_source : 1;
        switch ($channels_source)
        {
            case 1: // folder
                $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
                break;
            case 2: // url
                $channels_list_path = get_install_path();
                break;
            default:
                return array();
        }
        hd_print("Channels list search path: $channels_list_path");

        $all_channels = array();
        $list = glob($channels_list_path . '*.xml');
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename !== 'dune_plugin.xml') {
                hd_print("Found channels list: $filename");
                $all_channels[$filename] = $filename;
            }
        }

        if (empty($all_channels)) {
            hd_print("No channels list found in selected location: " . $channels_list_path);
            return $all_channels;
        }

        if (!in_array($used_list, $all_channels)) {
            $used_list = (string)reset($all_channels);
        }

        hd_print("Used channels list: $used_list");
        return $all_channels;
    }

    /**
     * @param Channel $a
     * @param Channel $b
     * @return int
     */
    public static function sort_channels_cb($a, $b)
    {
        // Sort by channel numbers.
        return strnatcasecmp($a->get_number(), $b->get_number());
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_epg_params($type)
    {
        return $this->EPG_PARSER_PARAMS[$type];
    }

    /**
     * @param array $val
     * @param string $type
     */
    public function set_epg_params($type, $val)
    {
        $this->EPG_PARSER_PARAMS[$type] = $val;
    }

    /**
     * @param string $param
     * @param mixed $val
     * @param string $type
     */
    public function set_epg_param($type, $param, $val)
    {
        $this->EPG_PARSER_PARAMS[$type][$param] = $val;
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_epg_param($param, $type)
    {
        return $this->EPG_PARSER_PARAMS[$type][$param];
    }

    public function try_reset_pages()
    {
        if ($this->is_entered) {
            $this->is_entered = false;
            $this->pages = array();
        }
    }

    public function reset_movie_counter()
    {
        $this->is_entered = true;
        $this->movie_counter = array();
    }

    /**
     * @param mixed $key
     * @return integer
     */
    public function get_movie_counter($key)
    {
        if (!array_key_exists($key, $this->movie_counter)) {
            $this->movie_counter[$key] = 0;
        }

        return $this->movie_counter[$key];
    }

    /**
     * @param mixed $key
     * @param integer $val
     */
    public function add_movie_counter($key, $val)
    {
        // entire list available, counter is final
        $this->movie_counter[$key] = $val;
    }

    /**
     * @param string $idx
     * @param int $increment
     * @return int
     */
    public function get_next_page($idx, $increment = 1)
    {
        if (!array_key_exists($idx, $this->pages)) {
            $this->pages[$idx] = 0;
        }

        $this->pages[$idx] += $increment;

        return $this->pages[$idx];
    }

    /**
     * @param string $idx
     * @param int $value
     */
    public function set_next_page($idx, $value)
    {
        $this->pages[$idx] = $value;
    }

    /**
     * @param string $name
     * @return mixed|null
     */
    public function get_filter($name)
    {
        return isset($this->filters[$name]) ? $this->filters[$name] : null;
    }

    /**
     * @param array $filters
     */
    public function set_filters($filters)
    {
        $this->filters = $filters;
    }

    /**
     * @return mixed|null
     */
    public function get_embedded_account()
    {
        return $this->embedded_account;
    }

    /**
     * @param object $account
     */
    public function set_embedded_account($account)
    {
        $this->embedded_account = $account;
    }

    /**
     * @return string
     */
    public function GET_BG_PICTURE()
    {
        return (string)$this->PLUGIN_BACKGROUND;
    }

    /**
     * @param array $defs
     * @param Starnet_Filter_Screen $parent
     * @param int $initial
     * @return bool
     */
    public function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        return false;
    }

    /**
     * @param array $user_input
     * @return string
     */
    public function CompileSaveFilterItem($user_input)
    {
        return '';
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        Control_Factory::add_label($defs, 'Баланс:', 'Информация о балансе не поддерживается');
    }

    /**
     * Update url macros {DOMAIN} and {TOKEN} by values from channel ext_params
     * Make url ts wrapped
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    abstract public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel);

    /**
     * Update url by channel ID (for correct hash calculation of url)
     * @param string $channel_id
     * @param array $ext_params
     * @return string
     */
    public function UpdateStreamUrlID($channel_id, $ext_params)
    {
        return str_replace('{ID}', $channel_id, $this->get_feature(MEDIA_URL_TEMPLATE_HLS));
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);
        $m3u_lines = $this->FetchTvM3U($plugin_cookies, $force);
        foreach ($m3u_lines as $line) {
            if (preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $line, $matches)) {
                return $matches;
            }
        }

        return false;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information for: $this->PLUGIN_SHOW_NAME");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $line, $matches)) {
                $pl_entries[$matches['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
        }

        return $pl_entries;
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearPlaylistCache()
    {
        $tmp_file = get_temp_path("playlist_tv.m3u8");
        hd_print("Clear playlist cache: $tmp_file");
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearChannelsCache($plugin_cookies)
    {
        $tmp_file = get_temp_path($plugin_cookies->channels_list);
        hd_print("Clear channels cache: $tmp_file");
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

            $search = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new Short_Movie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $params
     * @param $plugin_cookies
     * @return array
     */
    public function getFilterList($params, $plugin_cookies)
    {
        //hd_print("getFilterList: $params");
        return array();
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();
        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            $logo = $matches['logo'];
            $caption = $matches['title'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            $arr = explode("_", $query_id);
            $category_id = ($arr === false) ? $query_id : $arr[0];
            if ($category_id === $category) {
                $movies[] = new Short_Movie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies read: " . count($movies));
        return $movies;
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        return null;
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
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

    /**
     * @param string $url
     * @param $plugin_cookies
     * @return string
     */
    protected function UpdateMpegTsBuffering($url, $plugin_cookies)
    {
        if ($this->get_format($plugin_cookies) === 'mpeg') {
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        return HD::make_ts($url);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_format($plugin_cookies)
    {
        return isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
    }

    /**
     * @param $plugin_cookies
     * @param bool $force
     * @return array|false
     */
    protected function FetchTvM3U($plugin_cookies, $force = false)
    {
        $tmp_file = get_temp_path("playlist_tv.m3u8");
        if ($force !== false || !file_exists($tmp_file)) {
            try {
                $url = $this->GetPlaylistUrl('tv1', $plugin_cookies);
                //hd_print("tv1 m3u8 playlist: " . $url);
                if (empty($url)) {
                    hd_print("Tv1 playlist not defined");
                    throw new Exception('Tv1 playlist not defined');
                }
                file_put_contents($tmp_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                try {
                    $url = $this->GetPlaylistUrl('tv2', $plugin_cookies);
                    //hd_print("tv2 m3u8 playlist: " . $url);
                    if (empty($url)) {
                        throw new Exception("Tv2 playlist not defined");
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

    /**
     * @param $plugin_cookies
     * @param bool $force
     * @return array|false
     */
    public function FetchVodM3U($plugin_cookies, $force = false)
    {
        $m3u_file = get_temp_path("playlist_vod.m3u8");

        if ($force !== false || !file_exists($m3u_file)) {
            try {
                $url = $this->GetPlaylistUrl('movie', $plugin_cookies);
                if (empty($url)) {
                    hd_print('Vod playlist not defined');
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
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Starnet_Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        hd_print("Categories read: " . count($category_list));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        return '';
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => $this->FEATURES[SQUARE_ICONS] ? 60 : 84,
                    ViewItemParams::icon_height => $this->FEATURES[SQUARE_ICONS] ? 60 : 48,
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => $this->FEATURES[SQUARE_ICONS] ? 50 : 52,
                    ViewItemParams::icon_height => $this->FEATURES[SQUARE_ICONS] ? 50 : 34,
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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

            // 2x10 list view with title
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => $this->FEATURES[SQUARE_ICONS] ? 60 : 84,
                    ViewItemParams::icon_height => $this->FEATURES[SQUARE_ICONS] ? 60 : 48,
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
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
                ViewParams::item_detailed_info_title_color => 10,
                ViewParams::item_detailed_info_text_color => 15,
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
            PluginRegularFolderView::timer => array(GuiTimerDef::delay_ms => 5000),
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
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => $this->GET_BG_PICTURE(),
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

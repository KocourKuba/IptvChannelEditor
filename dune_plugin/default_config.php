<?php
require_once 'lib/epg_manager.php';
require_once 'lib/tv/channel.php';

abstract class DefaultConfig
{
    const BG_PICTURE_TEMPLATE = 'plugin_file://icons/bg_%s.jpg';
    const TMP_STORAGE = "/tmp/%s_%s";
    const VOD_PLAYLIST_NAME = 'vod_playlist.tmp';

    // info
    public static $PLUGIN_NAME = 'StarNet';
    public static $PLUGIN_SHORT_NAME = 'starnet';
    public static $PLUGIN_VERSION = '0.0.0';
    public static $PLUGIN_DATE = '04.01.1972';

    // supported features
    public static $TV_FAVORITES_SUPPORTED = true;
    public static $VOD_MOVIE_PAGE_SUPPORTED = false;
    public static $VOD_FAVORITES_SUPPORTED = false;

    // setup variables
    public static $MPEG_TS_SUPPORTED = false;
    public static $ACCOUNT_TYPE = 'UNKNOWN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = '';
    public static $ACCOUNT_PLAYLIST_URL2 = '';
    public static $STREAM_URL_PATTERN = '';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://online.dune-hd.com/demo/index.m3u8?channel=%s';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://online.dune-hd.com/demo/mpegts?channel=%s';
    public static $CHANNELS_LIST = 'default_channel_list.xml';
    public static $PLAY_LIST = 'playlist.m3u8';
    protected static $EPG1_URL_TEMPLATE = '';
    protected static $EPG2_URL_TEMPLATE = '';
    protected static $EPG1_PARSER = 'json';
    protected static $EPG2_PARSER = 'json';

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    public static $MOVIE_INFO_URL_TEMPLATE = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s'; // not used yet

    // page counter for some plugins
    protected static $pages = array();
    protected static $is_entered = false;
    protected static $movie_counter = array();

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

    protected static $VOD_CHANNEL_ICON_WIDTH = 190;
    protected static $VOD_CHANNEL_ICON_HEIGHT = 290;

    protected static $TV_CHANNEL_ICON_WIDTH = 84;
    protected static $TV_CHANNEL_ICON_HEIGHT = 48;

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
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        static::$movie_counter[$key] += $val;
    }

    public static function get_next_page($idx)
    {
        if (!array_key_exists($idx, static::$pages)) {
            static::$pages[$idx] = 0;
        }

        return ++static::$pages[$idx];
    }

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        if (!empty($plugin_cookies->subdomain_local) && !empty($plugin_cookies->ott_key_local)) {
            $subdomain = $plugin_cookies->subdomain_local;
            $token = $plugin_cookies->ott_key_local;
        } else if (!empty($plugin_cookies->subdomain) && !empty($plugin_cookies->ott_key)) {
            $subdomain = $plugin_cookies->subdomain;
            $token = $plugin_cookies->ott_key;
        } else {
            hd_print("token or subdomain not defined");
            return "";
        }

        $format = static::get_format($plugin_cookies);
        switch ($format) {
            case 'hls':
                $url = $channel->get_streaming_url();
                break;
            case 'mpeg':
                $url = static::$MEDIA_URL_TEMPLATE_MPEG;
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        if (intval($archive_ts) > 0) {
            if (strpos($url, '?') === false)
                $url .= '?';
            else
                $url .= '&';

            $now_ts = time();
            $url .= "utc=$archive_ts&lutc=$now_ts";
        }

        // hd_print("Stream type: " . $format);
        // hd_print("Stream url:  " . $url);
        // hd_print("Channel ID:  " . $channel->get_channel_id());
        // hd_print("Domain:      " . $subdomain);
        // hd_print("Token:       " . $token);
        // hd_print("Archive TS:  " . $archive_ts);

        $url = str_replace('{SUBDOMAIN}', $subdomain, $url);
        $url = str_replace('{ID}', $channel->get_channel_id(), $url);
        $url = str_replace('{TOKEN}', $token, $url);

        return static::make_ts($url);
    }

    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        return array();
    }

    public static function GetAccountInfo($plugin_cookies)
    {
        hd_print("Collect information from account " . static::$PLUGIN_NAME);

        $found = false;
        try {
            $content = self::FetchTemplatedUrl(static::$ACCOUNT_TYPE, static::$ACCOUNT_PLAYLIST_URL1, $plugin_cookies);
        } catch (Exception $ex) {
            try {
                $content = self::FetchTemplatedUrl(static::$ACCOUNT_TYPE, static::$ACCOUNT_PLAYLIST_URL2, $plugin_cookies);
            } catch (Exception $ex) {
                hd_print("Failed to fetch provider playlist");
                return false;
            }
        }

        $tmp_file = static::GET_TMP_STORAGE_PATH();
        file_put_contents($tmp_file, $content);
        $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        for ($i = 0; $i < count($lines); ++$i) {
            if (preg_match(static::$STREAM_URL_PATTERN, $lines[$i], $matches)) {

                $plugin_cookies->subdomain_local = $matches['subdomain'];
                // hd_print("domain: $plugin_cookies->subdomain_local");

                $plugin_cookies->ott_key_local = $matches['token'];
                // hd_print("token: $plugin_cookies->ott_key_local");

                if (isset($matches['host'])) {
                    $plugin_cookies->host = $matches['host'];
                    // hd_print("host: $plugin_cookies->host");
                }

                $found = true;
            }
        }
        unlink($tmp_file);

        return $found;
    }

    public static function getSearchList($keyword)
    {
        return array();
    }

    public static function getVideoList($idx)
    {
        return array();
    }

    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        return null;
    }

    public static function make_ts($url)
    {
        if (strpos($url, 'http://ts://') === false) {
            $url = str_replace('http://', 'http://ts://', $url);
        }

        return $url;
    }

    public static function get_format($plugin_cookies)
    {
        return isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
    }
    /**
     * @return array
     */
    public static function GetEPG(IChannel $channel, $day_start_ts)
    {
        try {
            if (empty(static::$EPG1_URL_TEMPLATE))
                throw new Exception("Empty first epg template");

            $epg = EpgManager::get_epg(static::$EPG1_PARSER,
                $channel,
                'first',
                $day_start_ts,
                static::$EPG1_URL_TEMPLATE,
                static::$PLUGIN_SHORT_NAME
            );
            if (count($epg) == 0)
                throw new Exception("Empty first epg");
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source " . $ex->getMessage());
                if (empty(static::$EPG2_URL_TEMPLATE))
                    throw new Exception("Empty second epg template");

                $epg = EpgManager::get_epg(static::$EPG2_PARSER,
                    $channel,
                    'second',
                    $day_start_ts,
                    static::$EPG2_URL_TEMPLATE,
                    static::$PLUGIN_SHORT_NAME
                );
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from secondary epg source: " . $ex->getMessage());
                $epg = array();
            }
        }

        hd_print("Loaded " . count($epg) . " EPG entries");

        return $epg;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
    }

    public static function LoadAndStoreJson($url, $to_array = true, $path = null)
    {
        try {
            $doc = HD::http_get_document($url);
            $categories = json_decode($doc, $to_array);
            if (empty($categories)) {
                hd_print("empty playlist or not valid token");
                return false;
            }

            if (!empty($path))
                file_put_contents($path, json_encode($categories));

        } catch (Exception $ex) {
            hd_print("Unable to load movie categories: " . $ex->getMessage());
            return false;
        }

        return $categories;
    }

    public static function LoadAndStoreM3U($url)
    {
        try {
            $doc = HD::http_get_document($url);
            if (empty($doc)) {
                hd_print("empty playlist or not valid token");
                return false;
            }

            file_put_contents(static::GET_VOD_TMP_STORAGE_PATH(), $doc);
        } catch (Exception $ex) {
            hd_print("Unable to load movie categories: " . $ex->getMessage());
            return false;
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    protected static function FetchTemplatedUrl($type, $template, $plugin_cookies)
    {
        hd_print("Type: $type");
        hd_print("Template: $template");

        if ($type == 'LOGIN') {
            $login = $plugin_cookies->login_local;
            if (empty($login)) {
                $login = $plugin_cookies->login;
            }

            $password = $plugin_cookies->password_local;
            if (empty($password)) {
                $password = $plugin_cookies->password;
            }

            if (empty($login) || empty($password))
                throw new Exception("Login or password not set");

            $url = sprintf($template, $login, $password);
        }
        else if ($type == 'PIN') {
            $password = $plugin_cookies->password_local;
            if (empty($password)) {
                $password = $plugin_cookies->password;
            }

            if (empty($password))
                throw new Exception("Password not set");

            $url = sprintf($template, $password);
        } else {
            throw new Exception("Unknown auth scheme");
        }

        return HD::http_get_document($url);
    }

    public static function GET_BG_PICTURE()
    {
        return sprintf(DefaultConfig::BG_PICTURE_TEMPLATE, static::$PLUGIN_SHORT_NAME);
    }

    protected static function GET_TV_ICON_WIDTH()
    {
        return static::$TV_CHANNEL_ICON_WIDTH;
    }

    protected static function GET_TV_ICON_HEIGHT()
    {
        return static::$TV_CHANNEL_ICON_HEIGHT;
    }

    protected static function GET_VOD_ICON_WIDTH()
    {
        return static::$VOD_CHANNEL_ICON_WIDTH;
    }

    protected static function GET_VOD_ICON_HEIGHT()
    {
        return static::$VOD_CHANNEL_ICON_HEIGHT;
    }

    /**
     * @return string
     */
    public static function GET_VOD_TMP_STORAGE_PATH()
    {
        return static::GET_TMP_STORAGE_PATH(DefaultConfig::VOD_PLAYLIST_NAME);
    }

    /**
     * @return string
     */
    public static function GET_TMP_STORAGE_PATH($name = null)
    {
        if ($name == null)
            $name = static::$PLAY_LIST;

        return sprintf(DefaultConfig::TMP_STORAGE, static::$PLUGIN_SHORT_NAME, $name);
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
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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

            array
            (
                // small with caption
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 3,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
                    //ViewParams::sandwich_width => 200,
                    //ViewParams::sandwich_height => 120,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => static::GET_TV_ICON_WIDTH(),
                    ViewItemParams::icon_height => static::GET_TV_ICON_HEIGHT(),
                    ViewItemParams::item_caption_width => 485,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
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
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_CHANNEL_ICON_PATH
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
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
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
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
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::TV_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::TV_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_CHANNEL_ICON_PATH,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            )
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
                    ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                    ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                    ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                    ViewParams::sandwich_width => DefaultConfig::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => DefaultConfig::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => static::GET_BG_PICTURE(),
                    ViewParams::background_order => 0,
                    ViewParams::background_height => 1080,
                    ViewParams::background_width => 1920,
                    ViewParams::optimize_full_screen_background => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => DefaultConfig::VOD_ICON_PATH,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => static::GET_VOD_ICON_WIDTH(),
                    ViewItemParams::icon_height => static::GET_VOD_ICON_HEIGHT(),
                    ViewItemParams::icon_sel_margin_top => 0,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::item_caption_width => 1100
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_width => static::GET_VOD_ICON_WIDTH(),
                    ViewItemParams::icon_height => static::GET_VOD_ICON_HEIGHT(),
                    ViewItemParams::icon_path => DefaultConfig::DEFAULT_MOV_ICON_PATH
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
                        ViewParams::background_path => static::GET_BG_PICTURE(),
                        ViewParams::background_order => 0,
                        ViewParams::background_height => 1080,
                        ViewParams::background_width => 1920,
                        ViewParams::optimize_full_screen_background => true,

                        ViewParams::paint_sandwich => false,
                        ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                        ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                        ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
                        ViewParams::sandwich_width => static::VOD_SANDWICH_WIDTH,
                        ViewParams::sandwich_height => static::VOD_SANDWICH_HEIGHT,
                        ViewParams::sandwich_icon_upscale_enabled => true,
                        ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ),

                    PluginRegularFolderView::not_loaded_view_item_params => array
                    (
                        ViewItemParams::item_paint_icon => true,
                        ViewItemParams::icon_width => static::GET_VOD_ICON_WIDTH(),
                        ViewItemParams::icon_height => static::GET_VOD_ICON_HEIGHT(),
                        //ViewItemParams::icon_width => 150,
                        //ViewItemParams::icon_height => 200,
                        ViewItemParams::icon_path => DefaultConfig::DEFAULT_MOV_ICON_PATH
                    ),

                    PluginRegularFolderView::base_view_item_params => array
                    (
                        ViewItemParams::item_paint_icon => true,
                        ViewItemParams::icon_sel_scale_factor => 1.2,
                        ViewItemParams::icon_path => DefaultConfig::VOD_ICON_PATH,
                        ViewItemParams::item_layout => HALIGN_LEFT,
                        ViewItemParams::icon_valign => VALIGN_CENTER,
                        ViewItemParams::icon_dx => 10,
                        ViewItemParams::icon_dy => -5,
                        ViewItemParams::icon_width => static::GET_VOD_ICON_WIDTH(),
                        ViewItemParams::icon_height => static::GET_VOD_ICON_HEIGHT(),
                        //ViewItemParams::icon_width => 150,
                        //ViewItemParams::icon_height => 200,
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
                        ViewParams::background_path => static::GET_BG_PICTURE(),
                        ViewParams::background_order => 0,
                        ViewParams::background_height => 1080,
                        ViewParams::background_width => 1920,
                        ViewParams::item_detailed_info_font_size => FONT_SIZE_NORMAL,

                        ViewParams::paint_sandwich => false,
                        ViewParams::sandwich_base => DefaultConfig::SANDWICH_BASE,
                        ViewParams::sandwich_mask => DefaultConfig::SANDWICH_MASK,
                        ViewParams::sandwich_cover => DefaultConfig::SANDWICH_COVER,
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
                        ViewItemParams::icon_path => DefaultConfig::DEFAULT_MOV_ICON_PATH
                    ),

                    PluginRegularFolderView::base_view_item_params => array
                    (
                        ViewItemParams::item_paint_icon => true,
                        ViewItemParams::icon_sel_scale_factor => 1.2,
                        ViewItemParams::icon_path => DefaultConfig::VOD_ICON_PATH,
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
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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
                    ViewParams::background_path => static::GET_BG_PICTURE(),
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

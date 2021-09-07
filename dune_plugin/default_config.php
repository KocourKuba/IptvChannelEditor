<?php
require_once 'lib/epg_manager.php';

abstract class DefaultConfig
{
    const BG_PICTURE_TEMPLATE = 'plugin_file://icons/bg_%s.jpg';
    const TMP_STORAGE = "/tmp/%s_%s";

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
    public static $USE_LOGIN_PASS = false;
    public static $USE_PIN = false;

    // tv
    public static $MEDIA_URL_TEMPLATE = 'http://ts://online.dune-hd.com/demo/index.m3u8?channel=%s';
    public static $MEDIA_URL_TEMPLATE_TS = 'http://ts://online.dune-hd.com/demo/mpegts?channel=%s';
    public static $CHANNELS_LIST = 'default_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://online.dune-hd.com/epg1/?channel=%s&date=%s';
    protected static $EPG2_URL_TEMPLATE = 'http://online.dune-hd.com/epg2/?channel=%s&date=%s';
    protected static $EPG1_PARSER = 'json';
    protected static $EPG2_PARSER = 'json';

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    public static $MOVIE_INFO_URL_TEMPLATE = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s'; // not used yet

    // EPG parsers html/json/xml

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

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, $url, $channel_id)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "&utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format) {
            case 'hls':
                $url = str_replace('{ID}', $channel_id, static::$MEDIA_URL_TEMPLATE);
                break;
            case 'mpeg':
                $url = str_replace('{ID}', $channel_id, static::$MEDIA_URL_TEMPLATE_TS);
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
        }

        return $url;
    }

    public static function getSearchList($keyword)
    {
        return array();
    }

    public static function getVideoList($category_id, $genre_id)
    {
        return array();
    }

    public static function GetAccountStatus($plugin_cookies)
    {
        return false;
    }

    public static function GetAccessInfo($plugin_cookies)
    {
        return false;
    }

    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        return null;
    }

    /**
     * @return array
     */
    public static function GetEPG(IChannel $channel, $day_start_ts)
    {
        try {
            $epg = EpgManager::get_epg(static::$EPG1_PARSER,
                $channel,
                'first',
                $day_start_ts,
                static::GET_EPG_URL_TEMPLATE('first'),
                static::$PLUGIN_SHORT_NAME
            );
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source");
                $epg = EpgManager::get_epg(static::$EPG1_PARSER,
                    $channel,
                    'second',
                    $day_start_ts,
                    static::GET_EPG_URL_TEMPLATE('second'),
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

    public static function GET_BG_PICTURE()
    {
        return sprintf(DefaultConfig::BG_PICTURE_TEMPLATE, static::$PLUGIN_SHORT_NAME);
    }

    public static function GET_EPG_URL_TEMPLATE($type)
    {
        switch ($type) {
            case 'first':
                return static::$EPG1_URL_TEMPLATE;
            case 'second':
                return static::$EPG2_URL_TEMPLATE;
        }

        return "";
    }

    public static function GET_TV_ICON_WIDTH()
    {
        return static::$TV_CHANNEL_ICON_WIDTH;
    }

    public static function GET_TV_ICON_HEIGHT()
    {
        return static::$TV_CHANNEL_ICON_HEIGHT;
    }

    public static function GET_VOD_ICON_WIDTH()
    {
        return static::$VOD_CHANNEL_ICON_WIDTH;
    }

    public static function GET_VOD_ICON_HEIGHT()
    {
        return static::$VOD_CHANNEL_ICON_HEIGHT;
    }

    /**
     * @return string
     */
    public static function GET_VOD_TMP_STORAGE_PATH()
    {
        return "/tmp/starnet_vod.json";
    }

    /**
     * @return string
     */
    public static function GET_TMP_STORAGE_PATH($name)
    {
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => DefaultConfig::GET_TV_ICON_WIDTH(),
                    ViewItemParams::icon_height => DefaultConfig::GET_TV_ICON_HEIGHT(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::sandwich_width => static::VOD_SANDWICH_WIDTH,
                    ViewParams::sandwich_height => static::VOD_SANDWICH_HEIGHT,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                    ViewParams::item_detailed_info_auto_line_break => true,
                    ViewParams::item_detailed_info_title_color => 10,
                    ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewItemParams::icon_width => DefaultConfig::GET_VOD_ICON_WIDTH(),
                    ViewItemParams::icon_height => DefaultConfig::GET_VOD_ICON_HEIGHT(),
                    ViewItemParams::icon_sel_margin_top => 0,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::item_caption_width => 1100
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::icon_width => DefaultConfig::GET_VOD_ICON_WIDTH(),
                    ViewItemParams::icon_height => DefaultConfig::GET_VOD_ICON_HEIGHT(),
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
                        ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                        ViewItemParams::icon_width => DefaultConfig::GET_VOD_ICON_WIDTH(),
                        ViewItemParams::icon_height => DefaultConfig::GET_VOD_ICON_HEIGHT(),
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
                        ViewItemParams::icon_width => DefaultConfig::GET_VOD_ICON_WIDTH(),
                        ViewItemParams::icon_height => DefaultConfig::GET_VOD_ICON_HEIGHT(),
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
                        ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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
                    ViewParams::background_path => DefaultConfig::GET_BG_PICTURE(),
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

<?php
require_once 'epg_manager.php';

abstract class DefaultConfig
{
    /** Not used */
    const VOD_CATEGORIES_URL = 'http://online.dune-hd.com/demo2/vod_categories.pl';
    const MOVIE_LIST_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    const MOVIE_INFO_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s';

    const VOD_MOVIE_PAGE_SUPPORTED = false;
    const VOD_FAVORITES_SUPPORTED = false;
    const TV_FAVORITES_SUPPORTED = true;

    const MPEG_TS_SUPPORTED = false;
    const USE_TOKEN = false;

    /** prefix to create cache file*/
    const BG_PICTURE_TEMPLATE = 'plugin_file://icons/bg_%s.jpg';

    const PLUGIN_NAME = 'StarNet';
    const PLUGIN_SHORT_NAME = 'starnet';
    const PLUGIN_VERSION = '0.0.0';
    const PLUGIN_DATE = '1972.1.1';

    const MEDIA_URL_TEMPLATE = 'http://online.dune-hd.com/demo/index.m3u8?channel=%s';
    const CHANNEL_LIST_URL = 'default_channel_list.xml';
    const EPG1_URL_FORMAT = 'http://online.dune-hd.com/epg1/?channel=%s&date=%s';
    const EPG2_URL_FORMAT = 'http://online.dune-hd.com/epg2/?channel=%s&date=%s';

    protected static $EPG_PARSER = 'json';
    protected static $TVG_PARSER = 'json';

    public abstract function AdjustStreamUri($plugin_cookies, $archive_ts, $url);
    public function GetAccessInfo($plugin_cookies)
    {
        return false;
    }

    public static function GetEPG(IChannel $channel, $day_start_ts)
    {
        try {
            $epg = EpgManager::get_epg(static::$EPG_PARSER,
                $channel,
                'first',
                $day_start_ts,
                static::GET_EPG_URL_FORMAT(1),
                static::GET_PLUGIN_SHORT_NAME()
            );
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source");
                $epg = EpgManager::get_epg(static::$EPG_PARSER,
                    $channel,
                    'second',
                    $day_start_ts,
                    static::GET_EPG_URL_FORMAT(2),
                    static::GET_PLUGIN_SHORT_NAME()
                );
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from secondary epg source: " . $ex->getMessage());
                $epg = array();
            }
        }

        hd_print("Loaded " . count($epg) . " EPG entries");

        return $epg;
    }

    public static function GET_BG_PICTURE() {
        return sprintf(static::BG_PICTURE_TEMPLATE, static::GET_PLUGIN_SHORT_NAME());
    }

    public static function GET_MPEG_TS_SUPPORTED() {
        return static::MPEG_TS_SUPPORTED;
    }

    public static function GET_USE_TOKEN() {
        return static::USE_TOKEN;
    }

    public static function GET_TV_FAVORITES_SUPPORTED() {
        return static::TV_FAVORITES_SUPPORTED;
    }

    public static function GET_VOD_MOVIE_PAGE_SUPPORTED() {
        return static::VOD_MOVIE_PAGE_SUPPORTED;
    }

    public static function GET_MOVIE_LIST_URL_FORMAT() {
        return static::MOVIE_LIST_URL_FORMAT;
    }

    public static function GET_VOD_FAVORITES_SUPPORTED() {
        return static::VOD_FAVORITES_SUPPORTED;
    }

    public static function GET_MOVIE_INFO_URL_FORMAT() {
        return static::MOVIE_INFO_URL_FORMAT;
    }

    public static function GET_VOD_CATEGORIES_URL() {
        return static::VOD_CATEGORIES_URL;
    }

    public static function GET_PLUGIN_NAME() {
        return static::PLUGIN_NAME;
    }

    public static function GET_PLUGIN_SHORT_NAME() {
        return static::PLUGIN_SHORT_NAME;
    }

    public static function GET_PLUGIN_VERSION() {
        return static::PLUGIN_VERSION;
    }

    public static function GET_PLUGIN_DATE() {
        return static::PLUGIN_DATE;
    }

    public static function GET_MEDIA_URL_TEMPLATE() {
        return static::MEDIA_URL_TEMPLATE;
    }

    public static function GET_CHANNEL_LIST_URL() {
        return static::CHANNEL_LIST_URL;
    }

    public static function GET_EPG_URL_FORMAT($type) {
        switch ($type)
        {
            case 1:
                return static::EPG1_URL_FORMAT;
            case 2:
                return static::EPG2_URL_FORMAT;
        }

        return  "";
    }
}

<?php
require_once 'tv/channel.php';

abstract class DefaultConfig
{
    /** Not used */
    const VOD_CATEGORIES_URL = 'http://online.dune-hd.com/demo2/vod_categories.pl';
    const MOVIE_LIST_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    const MOVIE_INFO_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s';

    const VOD_MOVIE_PAGE_SUPPORTED = false;
    const VOD_FAVORITES_SUPPORTED = false;
    const TV_FAVORITES_SUPPORTED = true;

    /** prefix to create cache file*/
    const EPG_CACHE_FILE_TEMPLATE = '%sepg_channel_%d_%d';
    const BG_PICTURE_TEMPLATE = 'plugin_file://bg_%s.jpg';
    const EPG_CACHE_DIR_TEMPLATE = '/tmp/%s_epg/';

    const PLUGIN_NAME = '';
    const PLUGIN_SHORT_NAME = '';
    const PLUGIN_VERSION = '';
    const PLUGIN_DATE = '';

    const MEDIA_URL_TEMPLATE = '';
    const CHANNEL_LIST_URL = '';
    const EPG1_URL_FORMAT = '';
    const EPG2_URL_FORMAT = '';

    protected static $EPG_PARSER = 'parse_epg_json';
    protected static $TVG_PARSER = 'parse_epg_json';

    public abstract function AdjustStreamUri($plugin_cookies, $archive_ts, $url);

    public static function LoadCachedEPG(IChannel $channel, $day_start_ts, &$epg)
    {
        if (!is_dir(self::GET_EPG_CACHE_DIR())) {
            mkdir(self::GET_EPG_CACHE_DIR());
        }

        $cache_file = self::GET_CACHE_FILE($channel->get_id(), $day_start_ts);
        if (!file_exists($cache_file))
            return false;

        hd_print("Load EPG from cache: $cache_file");
        $epg = unserialize(file_get_contents($cache_file));
        return true;
    }

    public static function GetEPG(IChannel $channel, $day_start_ts)
    {
        $epg = array();
        $epg_date = gmdate("Ymd", $day_start_ts); // 'YYYYMMDD'
        try {
            $epg_id = $channel->get_epg_id();
            if (empty($epg_id)) {
                throw new Exception("EPG not defined for channel" . $channel->get_title() . "'");
            }
            hd_print("Fetching EPG ID from primary epg source: '$epg_id' DATE: $epg_date");
            $parser = static::$EPG_PARSER;
            $epg = HD::$parser(sprintf(static::GET_EPG_URL_FORMAT(1), $epg_id, $epg_date), $day_start_ts);
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source");
                $tvg_id = $channel->get_tvg_id();
                if (empty($tvg_id)) {
                    throw new Exception("EPG not defined for channel" . $channel->get_title() . "'");
                }
                hd_print("Fetching EPG ID from secondary epg source: '$tvg_id' DATE: $epg_date");
                $parser = static::$TVG_PARSER;
                $epg = HD::$parser(sprintf(static::GET_EPG_URL_FORMAT(2), $tvg_id, $epg_date), $day_start_ts);
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from secondary epg source: " . $ex->getMessage());
                return $epg;
            }
        }

        DefaultConfig::SortAndStore($channel, $day_start_ts, $epg);

        return $epg;
    }

    protected static function SortAndStore(IChannel $channel, $day_start_ts, $epg)
    {
        // sort epg by date
        $counts = count($epg);
        $cache_file = self::GET_CACHE_FILE($channel->get_id(), $day_start_ts);
        hd_print("Save EPG to cache: $cache_file $counts entries");

        if ($counts > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents(self::GET_CACHE_FILE($channel->get_id(), $day_start_ts), serialize($epg));
        }
    }

    protected static function GET_CACHE_FILE($id, $day_start_ts) {
        return sprintf(self::EPG_CACHE_FILE_TEMPLATE, self::GET_EPG_CACHE_DIR(), $id, $day_start_ts);
    }

    protected static function GET_EPG_CACHE_DIR() {
        return sprintf(self::EPG_CACHE_DIR_TEMPLATE, static::GET_PLUGIN_SHORT_NAME());
    }

    public static function GET_BG_PICTURE() {
        return sprintf(self::BG_PICTURE_TEMPLATE, static::GET_PLUGIN_SHORT_NAME());
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

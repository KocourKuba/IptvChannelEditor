<?php
require_once 'config.php';
require_once 'tv/channel.php';

abstract class DefaultConfig implements IConfig
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
    const EPG_URL_FORMAT = '';
    const TVG_URL_FORMAT = '';
    const TVG_PROVIDER = '';
    const EPG_PROVIDER = '';

    public function LoadCachedEPG(IChannel $channel, $day_start_ts, &$epg)
    {
        if (!is_dir($this->get_epg_cache_dir())) {
            mkdir($this->get_epg_cache_dir());
        }

        $cache_file = $this->get_cache_file($channel->get_id(), $day_start_ts);
        if (!file_exists($cache_file))
            return false;

        hd_print("Load EPG from cache: $cache_file");
        $epg = unserialize(file_get_contents($cache_file));
        return true;
    }

    public function SortAndStore(IChannel $channel, $day_start_ts, $epg)
    {
        // sort epg by date
        $counts = count($epg);
        $cache_file = $this->get_cache_file($channel->get_id(), $day_start_ts);
        hd_print("Save EPG to cache: $cache_file $counts entries");

        if ($counts > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents($this->get_cache_file($channel->get_id(), $day_start_ts), serialize($epg));
        }
    }

    protected function get_cache_file($id, $day_start_ts) {
        return sprintf(self::EPG_CACHE_FILE_TEMPLATE, $this->get_epg_cache_dir(), $id, $day_start_ts);
    }

    public function GET_TV_FAVORITES_SUPPORTED() {
        return static::TV_FAVORITES_SUPPORTED;
    }

    public function GET_VOD_MOVIE_PAGE_SUPPORTED() {
        return static::VOD_MOVIE_PAGE_SUPPORTED;
    }

    public function GET_MOVIE_LIST_URL_FORMAT() {
        return static::MOVIE_LIST_URL_FORMAT;
    }

    public function GET_VOD_FAVORITES_SUPPORTED() {
        return static::VOD_FAVORITES_SUPPORTED;
    }

    public function GET_MOVIE_INFO_URL_FORMAT() {
        return static::MOVIE_INFO_URL_FORMAT;
    }

    public function GET_VOD_CATEGORIES_URL() {
        return static::VOD_CATEGORIES_URL;
    }

    public function GET_PLUGIN_NAME() {
        return static::PLUGIN_NAME;
    }

    public function GET_PLUGIN_SHORT_NAME() {
        return static::PLUGIN_SHORT_NAME;
    }

    public function GET_PLUGIN_VERSION() {
        return static::PLUGIN_VERSION;
    }

    public function GET_PLUGIN_DATE() {
        return static::PLUGIN_DATE;
    }

    public function get_bg_picture() {
        return sprintf(self::BG_PICTURE_TEMPLATE, $this->GET_PLUGIN_SHORT_NAME());
    }

    public function GET_MEDIA_URL_TEMPLATE() {
        return static::MEDIA_URL_TEMPLATE;
    }

    public function GET_CHANNEL_LIST_URL() {
        return static::CHANNEL_LIST_URL;
    }

    public function GET_EPG_URL_FORMAT() {
        return static::EPG_URL_FORMAT;
    }

    public function GET_TVG_URL_FORMAT() {
        return static::TVG_URL_FORMAT;
    }

    public function get_epg_cache_dir() {
        return sprintf(self::EPG_CACHE_DIR_TEMPLATE, $this->GET_PLUGIN_SHORT_NAME());
    }

    public function GET_EPG_PROVIDER() {
        return static::EPG_PROVIDER;
    }

    public function GET_TVG_PROVIDER() {
        return static::TVG_PROVIDER;
    }
}

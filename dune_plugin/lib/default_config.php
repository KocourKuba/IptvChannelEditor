<?php

abstract class DefaultConfig
{
    /** prefix to create cache file*/
    public $EPG_CACHE_FILE = 'epg_channel_';

    /** Not used */
    public $VOD_CATEGORIES_URL = 'http://online.dune-hd.com/demo2/vod_categories.pl';
    public $MOVIE_LIST_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    public $MOVIE_INFO_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s';

    public $PluginName = 'StarnetTV';
    public $PluginVersion = '0.0.0';
    public $PluginDate = '27.07.2021';
    public $VOD_MOVIE_PAGE_SUPPORTED = false;
    public $VOD_FAVORITES_SUPPORTED = false;
    public $TV_FAVORITES_SUPPORTED = true;

    public $MEDIA_URL_TEMPLATE = 'http://{ID}/index.m3u8';
    public $CHANNEL_LIST_URL = 'channel_list.xml';
    public $EPG_URL_FORMAT = 'http://online.dune-hd.com/epg/%d.json';
    public $TVG_URL_FORMAT = 'http://online.dune-hd.com/%d.html';
    public $EPG_CACHE_DIR = '/tmp/epg/';
    public $TVG_PROVIDER = 'unknown';
    public $EPG_PROVIDER = 'unknown';
    public $BG_PICTURE = '';

    abstract public function AdjustStreamUri($plugin_cookies, $archive_ts, $url);

    public function create_cache_folder()
    {
        if (!is_dir($this->EPG_CACHE_DIR)) {
           mkdir($this->EPG_CACHE_DIR);
        }
    }

    public static function LoadCachedEPG($cache_file, &$epg)
    {
        if (!file_exists($cache_file))
            return false;

        hd_print("Load EPG from cache: $cache_file");
        $epg = unserialize(file_get_contents($cache_file));
        return true;
    }
}

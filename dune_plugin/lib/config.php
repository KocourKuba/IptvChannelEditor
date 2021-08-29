<?php

interface IConfig
{
    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url);
    public function GetEPG(IChannel $channel, $day_start_ts);
    public function LoadCachedEPG(IChannel $channel, $day_start_ts, &$epg);
    public function SortAndStore(IChannel $channel, $day_start_ts, $epg);
    public function get_bg_picture();
    public function get_epg_cache_dir();

    public function GET_TV_FAVORITES_SUPPORTED();
    public function GET_VOD_MOVIE_PAGE_SUPPORTED();
    public function GET_VOD_FAVORITES_SUPPORTED();
    public function GET_MOVIE_INFO_URL_FORMAT();
    public function GET_MOVIE_LIST_URL_FORMAT();
    public function GET_VOD_CATEGORIES_URL();

    public function GET_PLUGIN_NAME();
    public function GET_PLUGIN_VERSION();
    public function GET_PLUGIN_DATE();

    public function GET_MEDIA_URL_TEMPLATE();
    public function GET_CHANNEL_LIST_URL();
    public function GET_EPG_URL_FORMAT();
    public function GET_TVG_URL_FORMAT();
    public function GET_EPG_PROVIDER();
    public function GET_TVG_PROVIDER();
}

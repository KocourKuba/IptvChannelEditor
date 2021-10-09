<?php
require_once 'default_config.php';

class ItvPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://api.itv.live/data/%s';

    // info
    public static $PLUGIN_NAME = 'ITV Live TV';
    public static $PLUGIN_SHORT_NAME = 'itv';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '12.10.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = false;
    public static $VOD_FAVORITES_SUPPORTED = false;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'https://itv.ooo/p/%s/hls.m3u8';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/live/(?<token>.+)/.+/.+\.m3u8$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}}//video.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $CHANNELS_LIST = 'itv_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.itv.live/epg/ch001';

    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        // this account has special API to get account info
        if (empty($plugin_cookies->password))
            return false;

        try {
            $url = sprintf(static::ACCOUNT_INFO_URL1, $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            return false;
        }

        parent::GetAccountInfo($plugin_cookies, $account_data, $force);

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"));
        if (isset($account_data->package_info) && !empty($account_data->package_info)) {
            return $account_data->status == 'ok';
        }

        return false;
    }
}

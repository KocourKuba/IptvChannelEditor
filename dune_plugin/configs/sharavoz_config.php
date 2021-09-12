<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '09.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/.*/.+\.m3u8\?token=(?<token>.+)$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $CHANNELS_LIST = 'sharavoz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public static function GetAccountStatus($plugin_cookies)
    {
        return static::GetAccountStreamInfo($plugin_cookies);
    }
}

<?php
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'Glanz TV';
    public static $PLUGIN_SHORT_NAME = 'glanz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '07.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $USE_LOGIN_PASS = true;

    // account
    public static $ACCOUNT_PLAYLIST_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/\d+/.+\.m3u8)\?username=.+&password=.+&token=(?<token>.+)&ch_id=\d+&req_host=(?<host>.+)$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $CHANNELS_LIST = 'glanz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::AdjustStreamUri($plugin_cookies, $archive_ts, $channel);

        $url = str_replace('{LOGIN}', $plugin_cookies->login, $url);
        $url = str_replace('{PASSWORD}', $plugin_cookies->password, $url);
        return str_replace('{HOST}', $plugin_cookies->host, $url);
    }
}

<?php
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'Glanz TV';
    public static $PLUGIN_SHORT_NAME = 'glanz';
    public static $PLUGIN_VERSION = '1.0.1';
    public static $PLUGIN_DATE = '12.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/\d+/.+\.m3u8\?username=.+&password=.+&token=(?<token>.+)&ch_id=\d+&req_host=(?<host>.+)$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/video-{START}-10800.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $CHANNELS_LIST = 'glanz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        if (empty($plugin_cookies->subdomain_local) || empty($plugin_cookies->ott_key_local)) {
            hd_print("token or subdomain not defined");
            return "";
        }

        $url = $channel->get_streaming_url();
        $id = $channel->get_channel_id();
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';

        switch ($format) {
            case 'hls':
                if (intval($archive_ts) > 0) {
                    $url = self::$MEDIA_URL_TEMPLATE_ARCHIVE_HLS;
                }
                break;
            case 'mpeg':
                if (intval($archive_ts) > 0) {
                    $url = self::$MEDIA_URL_TEMPLATE_ARCHIVE_MPEG;
                } else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        // hd_print("Stream type: " . $format);
        // hd_print("Stream url:  " . $url);
        // hd_print("Channel ID:  " . $id);
        // hd_print("Domain:      " . $plugin_cookies->subdomain_local);
        // hd_print("Token:       " . $plugin_cookies->ott_key_local);
        // hd_print("Int ID:      " . $channel->get_number());
        // hd_print("Host:        " . $plugin_cookies->host);
        // hd_print("Archive TS:  " . $archive_ts);

        $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain_local, $url);
        $url = str_replace('{ID}', $id, $url);
        $url = str_replace('{START}', $archive_ts, $url);
        $url = str_replace('{TOKEN}', $plugin_cookies->ott_key_local, $url);
        $url = str_replace('{LOGIN}', $plugin_cookies->login, $url);
        $url = str_replace('{PASSWORD}', $plugin_cookies->password, $url);
        $url = str_replace('{INT_ID}', strval($channel->get_number()), $url);
        return str_replace('{HOST}', $plugin_cookies->host, $url);
    }

    public static function GetAccountStatus($plugin_cookies)
    {
        return static::GetAccountStreamInfo($plugin_cookies);
    }
}

<?php
require_once 'default_config.php';

class AntifrizPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'AntiFriz TV';
    public static $PLUGIN_SHORT_NAME = 'antifriz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '07.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $USE_PIN = true;

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://antifriz.tv/playlist/%s.m3u8';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/s/(?<token>.+)/.+/video\.m3u8$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}';
    public static $CHANNELS_LIST = 'antifriz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id date(YYYYMMDD)

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        if (empty($plugin_cookies->subdomain_local) || empty($plugin_cookies->ott_key_local))
            return  "";

        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        $domain = explode(':', $plugin_cookies->subdomain_local);

        switch ($format) {
            case 'hls':
                $subdomain = $plugin_cookies->subdomain_local;
                if ($archive_ts) {
                    $url = self::$MEDIA_URL_TEMPLATE_ARCHIVE_HLS;
                    $subdomain = $domain[0];
                } else {
                    $url = $channel->get_streaming_url();
                }
                break;
            case 'mpeg':
                $subdomain = $domain[0];
                $url = self::$MEDIA_URL_TEMPLATE_ARCHIVE_MPEG;
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        $url = str_replace('{ID}', $channel->get_channel_id(), $url);
        $url = str_replace('{START}', $archive_ts, $url);
        $url = str_replace('{SUBDOMAIN}', $subdomain, $url);
        $url = str_replace('{TOKEN}', $plugin_cookies->ott_key_local, $url);

        if (strpos($url, 'http://ts://') === false) {
            $url = str_replace('http://', 'http://ts://', $url);
        }

        return $url;
    }
}

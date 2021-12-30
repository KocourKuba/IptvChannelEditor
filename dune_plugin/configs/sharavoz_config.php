<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'PIN';
    public static $MPEG_TS_SUPPORTED = true;

    // tv
    protected static $PLAYLIST_TV_URL = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/(?:.*)\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYY-MM-DD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('index.m3u8', 'mpegts', $url);
        }

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
        }

        return sprintf(self::$PLAYLIST_TV_URL, $password);
    }
}

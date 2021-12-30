<?php
require_once 'default_config.php';

class OttclubPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'PIN';

    // tv
    protected static $PLAYLIST_TV_URL = 'http://myott.top/playlist/%s/m3u';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/stream/(?<token>.+)/(?<id>.+)\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/stream/{TOKEN}/{ID}.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://myott.top/api/channel/%s'; // epg_id

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
        // hd_print("Stream url:  " . $url);
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

<?php
require_once 'default_config.php';

class SharatvPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'LOGIN';

    // tv
    protected static $PLAYLIST_TV_URL = 'http://tvfor.pro/g/%s:%s/1/playlist.m3u';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/{ID}/{TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/shara-tv/epg/%s.json'; // epg_id

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

        // shara tv does not support hls, only mpeg-ts
        // hd_print("Stream url:  " . $url);
        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        return sprintf(self::$PLAYLIST_TV_URL, $login, $password);
    }
}

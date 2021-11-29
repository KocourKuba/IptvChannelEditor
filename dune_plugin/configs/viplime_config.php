<?php
require_once 'default_config.php';

class ViplimePluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'PIN';
    public static $MPEG_TS_SUPPORTED = true;

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/viplime/epg/%s.json'; // epg_id

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

        $ext_params = $channel->get_ext_params();
        if (isset($ext_params['quality'])) {
            $url = str_replace('{QUALITY}', $ext_params['quality'], $url);
        }

        //hd_print("AdjustStreamUrl: $url");

        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            // replace hls to mpegts
            $url = str_replace('.m3u8', '.mpeg', $url);
            $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    protected static function GetTemplatedUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
        }

        return sprintf('http://cdntv.online/high/%s/playlist.m3u8', $password);
    }
}

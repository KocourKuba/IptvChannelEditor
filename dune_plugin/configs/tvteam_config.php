<?php
require_once 'default_config.php';

class TvteamPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'PIN';
    public static $MPEG_TS_SUPPORTED = true;

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/mono\.m3u8\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/mono.m3u8?token={TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/tvteam/epg/%s.json'; // epg_id

    // Views variables
    protected static $TV_CHANNEL_ICON_WIDTH = 60;
    protected static $TV_CHANNEL_ICON_HEIGHT = 60;

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
        //hd_print("AdjustStreamUrl: $url");

        switch (self::get_format($plugin_cookies))
        {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('mono.m3u8', "index-$archive_ts-7200.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('mono.m3u8', "archive-$archive_ts-7200.ts", $url);
                }
                else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }

                $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
                break;
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
        }

        return sprintf('http://tv.team/pl/11/%s/playlist.m3u8', $password);
    }
}

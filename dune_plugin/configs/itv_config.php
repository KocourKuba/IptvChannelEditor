<?php
require_once 'default_config.php';

class ItvPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'PIN';
    public static $MPEG_TS_SUPPORTED = true;

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/[^\?]+\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://api.itv.live/epg/%s/%s'; // epg_id date(YYYY-MM-DD)

    // Views variables
    protected static $TV_CHANNEL_ICON_WIDTH = 60;
    protected static $TV_CHANNEL_ICON_HEIGHT = 60;

    public function __construct()
    {
        parent::__construct();
        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'res';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'startTime';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'stopTime';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'desc';
    }

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

        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            // replace hls to mpegts
            $url = str_replace('video.m3u8', 'mpegts', $url);
            $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
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

        return sprintf('https://itv.ooo/p/%s/hls.m3u8', $password);
    }

    /**
     * Get information from the provider
     * @param $plugin_cookies
     * @param array &$account_data
     * @param bool $force - ignored
     * @return bool true if information collected and packages exists
     */
    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        // this account has special API to get account info
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if ($force === false && !empty($password)) {
            return true;
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $url = sprintf('http://api.itv.live/data/%s', $password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            return false;
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"), true);
        return isset($account_data['package_info']) && !empty($account_data['package_info']);
    }
}

<?php
require_once 'default_config.php';

class ItvPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://api.itv.live/data/%s';

    // info
    public static $PLUGIN_SHOW_NAME = 'ITV Live TV';
    public static $PLUGIN_SHORT_NAME = 'itv';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '11.10.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'https://itv.ooo/p/%s/hls.m3u8';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/[^\?]+\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/video.m3u8?token={TOKEN}';
    public static $CHANNELS_LIST = 'itv_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.itv.live/epg/%s/%s'; // epg_id YYYY-MM-DD

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
    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();
        //hd_print("AdjustStreamUrl: $url");

        if (intval($archive_ts) > 0) {
            $now_ts = strval(time());
            $url .= "&utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        if ($format == 'mpeg') {
            // replace hls to mpegts
            $url = str_replace('video.m3u8', 'mpegts', $url);
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
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
        if (empty($plugin_cookies->password))
            return false;

        try {
            $url = sprintf(static::ACCOUNT_INFO_URL1, $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            return false;
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"), true);
        return isset($account_data['package_info']) && !empty($account_data['package_info']);
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        return parent::GetPlaylistStreamInfo($plugin_cookies);
    }

    /**
     * Update url by provider additional parameters
     * @param $channel_id
     * @param $plugin_cookies
     * @param $ext_params
     * @return string
     */
    public static function UpdateStreamUri($channel_id, $plugin_cookies, $ext_params)
    {
        // itv token unique for each channel
        $url = str_replace('{ID}', $channel_id, static::$MEDIA_URL_TEMPLATE_HLS);
        $url = str_replace('{SUBDOMAIN}', $ext_params['subdomain'], $url);
        $url = str_replace('{TOKEN}', $ext_params['token'], $url);
        return static::make_ts($url);
    }
}

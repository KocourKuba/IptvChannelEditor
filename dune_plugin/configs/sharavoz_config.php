<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.1';
    public static $PLUGIN_DATE = '12.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.*)/(?:.*)\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $CHANNELS_LIST = 'sharavoz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public function __construct()
    {
        parent::__construct();
        static::$EPG_PARSER_PARAMS['epg_root'] = 'epg_data';
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

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "&utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now          " . $now_ts);
        }

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        if ($format == 'mpeg') {
            $url = str_replace('index.m3u8', 'mpegts', $url);
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

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
        $url = str_replace('{ID}', $channel_id, static::$MEDIA_URL_TEMPLATE_HLS);
        $url = str_replace('{SUBDOMAIN}', $ext_params['subdomain'], $url);
        $url = str_replace('{TOKEN}', $ext_params['token'], $url);
        return static::make_ts($url);
    }
}

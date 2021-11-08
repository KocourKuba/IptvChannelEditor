<?php
require_once 'default_config.php';

class ViplimePluginConfig extends DefaultConfig
{
    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://cdntv.online/high/%s/playlist.m3u8';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<quality>)/(?<token>.+)/(?<id>.+)\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8';
    public static $CHANNELS_LIST = 'viplime_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/viplime/epg/%s.json'; // epg_id

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

        if ((int)$archive_ts > 0) {
            $now_ts = (string)time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        if ($format === 'mpeg') {
            // replace hls to mpegts
            $url = str_replace('.m3u8', '.mpeg', $url);
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
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
        $url = parent::UpdateStreamUri($channel_id, $plugin_cookies, $ext_params);
        $url = str_replace('{QUALITY}', $ext_params['quality'], $url);
        return static::make_ts($url);
    }
}

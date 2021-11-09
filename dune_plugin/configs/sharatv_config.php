<?php
require_once 'default_config.php';

class SharatvPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://tvfor.pro/g/%s:%s/1/playlist.m3u';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/{TOKEN}';
    public static $CHANNELS_LIST = 'sharatv_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/shara-tv/epg/%s.json'; // epg_id

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

        if ((int)$archive_ts > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        // shara tv does not support hls, only mpeg-ts
        $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
        $url .= "|||dune_params|||buffering_ms:$buf_time";

        // hd_print("Stream url:  " . $url);

        return $url;
    }
}

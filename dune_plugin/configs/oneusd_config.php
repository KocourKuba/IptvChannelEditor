﻿<?php
require_once 'default_config.php';

class OneusdPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://api.itv.live/data/%s';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://1usd.tv/pl-%s-hls';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/mono\.m3u8\?token=(?<token>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/mono.m3u8?token={TOKEN}';
    public static $CHANNELS_LIST = 'oneusd_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/tvteam/epg/%s.json'; // epg_id

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

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        switch ($format)
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

                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
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
        // itv token unique for each channel
        $url = str_replace(
            array('{ID}', '{SUBDOMAIN}', '{TOKEN}'),
            array($channel_id, $ext_params['subdomain'], $ext_params['token']),
            static::$MEDIA_URL_TEMPLATE_HLS);
        return static::make_ts($url);
    }
}

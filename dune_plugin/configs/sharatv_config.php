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

    // Views variables
    protected static $TV_CHANNEL_ICON_WIDTH = 84;
    protected static $TV_CHANNEL_ICON_HEIGHT = 48;

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
        $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);

        // hd_print("Stream url:  " . $url);

        return $url;
    }
}

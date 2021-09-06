<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '01.09.2021';

    public static $MPEG_TS_SUPPORTED = true;
    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $CHANNELS_LIST = 'sharavoz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "&utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format) {
            case 'hls':
                break;
            case 'mpeg':
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url = str_replace('index.m3u8', 'mpegts', $url);
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
        }

        return $url;
    }
}

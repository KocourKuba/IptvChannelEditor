<?php
require_once 'lib/default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '01.09.2021';

    public static $MPEG_TS_SUPPORTED = true;

    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $CHANNEL_LIST_URL = 'sharavoz_channel_list.xml';
    public static $EPG1_URL_FORMAT = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    public static $EPG2_URL_FORMAT = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "&utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format)
        {
            case 'hls':
                break;
            case 'mpeg':
                $url = str_replace('index.m3u8', 'mpegts', $url);
                break;
        }

        return $url;
    }
}

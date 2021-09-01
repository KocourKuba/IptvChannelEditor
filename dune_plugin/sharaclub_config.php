<?php
require_once 'lib/default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const PLUGIN_NAME = 'Sharaclub TV';
    const PLUGIN_SHORT_NAME = 'sharaclub';
    const PLUGIN_VERSION = '1.0.0';
    const PLUGIN_DATE = '01.09.2021';

    const MPEG_TS_SUPPORTED = true;

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    const CHANNEL_LIST_URL = 'sharaclub_channel_list.xml';
    const EPG1_URL_FORMAT = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)
    const EPG2_URL_FORMAT = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format)
        {
            case 'hls':
                break;
            case 'mpeg':
                $url = str_replace('/video.m3u8', '.ts', $url);
                break;
        }

        return $url;
    }
}

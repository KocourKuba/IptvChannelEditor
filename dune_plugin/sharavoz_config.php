<?php
require_once 'lib/default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    const PLUGIN_NAME = 'Sharavoz TV';
    const PLUGIN_SHORT_NAME = 'sharavoz';
    const PLUGIN_VERSION = '1.0.0';
    const PLUGIN_DATE = '28.08.2021';

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    const CHANNEL_LIST_URL = 'sharavoz_channel_list.xml';
    const EPG1_URL_FORMAT = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    const EPG2_URL_FORMAT = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: stream $format");
        switch ($format)
        {
            case 'hls':
                $url = str_replace('mpegts', 'index.m3u8', $url);
                if ($archive_ts > 0) {
                    $url = str_replace("index.m3u8", "archive-" . $archive_ts . "-10800.m3u8", $url);
                }
                break;
            case 'mpeg':
                $url = str_replace('index.m3u8', 'mpegts', $url);
                $url = str_replace('http://', 'http://ts://', $url);
                $url = str_replace('http://ts://mp4://', 'http://mp4://', $url);
                if ($archive_ts > 0) {
                    $url = str_replace("mpegts", "archive-" . $archive_ts . "-10800.ts", $url);
                }
                break;
        }

        hd_print("AdjustStreamUri: $url");
        return $url;
    }
}

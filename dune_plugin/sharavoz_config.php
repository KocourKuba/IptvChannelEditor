<?php
require_once 'lib/default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        parent::__construct();

        $this->PluginName = 'Sharavoz TV';
        $this->PluginVersion = '1.0.0';
        $this->PluginDate = '27.07.2021';

        $this->MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={UID}';
        $this->CHANNEL_LIST_URL = 'sharavoz_channel_list.xml';
        $this->EPG_URL_FORMAT = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s';
        $this->TVG_URL_FORMAT = 'http://api.program.spr24.net/api/program?epg=%s&date=%s';
        $this->EPG_CACHE_DIR = '/tmp/sharavoz_epg/';
    }

    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("stream $format");
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

        return $url;
    }
}

?>

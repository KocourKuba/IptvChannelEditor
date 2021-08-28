<?php
require_once 'lib/default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        $this->PluginName = 'Sharaclub TV';
        $this->PluginVersion = '1.0.0';
        $this->PluginDate = '28.08.2021';

        $this->BG_PICTURE = 'plugin_file://bg_sharaclub.jpg';
        $this->MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
        $this->CHANNEL_LIST_URL = 'sharaclub_channel_list.xml';
        $this->EPG_URL_FORMAT = 'https://list.playtv.pro/f/epg.xml.gz';
        $this->TVG_URL_FORMAT = 'https://list.playtv.pro/f/epg.xml.gz';
        $this->EPG_CACHE_DIR = '/tmp/sharaclub_epg/';
        $this->TVG_PROVIDER = 'sharaclub';
        $this->EPG_PROVIDER = 'sharaclub';
    }

    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        if ($archive_ts > 0) {
            $url = str_replace("index.m3u8", "archive-" . $archive_ts . "-10800.m3u8", $url);
        }

        hd_print("AdjustStreamUri: $url");
        return $url;
    }
}

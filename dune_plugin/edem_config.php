<?php
require_once 'lib/default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        $this->PluginName = 'iEdem/iLook TV';
        $this->PluginVersion = '2.6.0';
        $this->PluginDate = '28.08.2021';

        $this->BG_PICTURE = 'plugin_file://bg_edem.jpg';
        $this->MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
        $this->CHANNEL_LIST_URL = 'edem_channel_list.xml';
        $this->EPG_URL_FORMAT = 'http://epg.ott-play.com/edem/epg/%d.json';
        $this->TVG_URL_FORMAT = 'http://www.teleguide.info/kanal%d_%s.html';
        $this->EPG_CACHE_DIR = '/tmp/edem_epg/';
        $this->TVG_PROVIDER = 'teleguide';
        $this->EPG_PROVIDER = 'ott-play';
    }

    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        if (strpos($url, 'http://ts://') === false) {
            $url = str_replace('http://', 'http://ts://', $url);
        }

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            if (strpos($url, '?') === false)
                $url .= '?';
            else
                $url .= '&';

            $url .= "utc=$archive_ts&lutc=$now_ts";
        }

        hd_print("AdjustStreamUri: $url");
        return $url;
    }
}

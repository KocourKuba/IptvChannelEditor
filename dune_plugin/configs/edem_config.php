<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        $this->PLUGIN_NAME = 'iEdem/iLook TV';
        $this->PLUGIN_SHORT_NAME = 'edem';
        $this->PLUGIN_VERSION = '2.6.0';
        $this->PLUGIN_DATE = '01.09.2021';

        $this->MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
        $this->CHANNEL_LIST_URL = 'edem_channel_list.xml';
        $this->EPG1_URL_FORMAT = 'http://epg.ott-play.com/edem/epg/%s.json'; // epg_id
        $this->EPG2_URL_FORMAT = 'http://www.teleguide.info/kanal%s_%s.html'; // epg_id date(YYYYMMDD)

        $this->TVG_PARSER = 'html';
    }

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        if (intval($archive_ts) > 0) {
            $now_ts = time();
            if (strpos($url, '?') === false)
                $url .= '?';
            else
                $url .= '&';

            $url .= "utc=$archive_ts&lutc=$now_ts";
        }

        return $url;
    }
}

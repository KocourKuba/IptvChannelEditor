<?php
require_once 'lib/default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    const PLUGIN_NAME = 'iEdem/iLook TV';
    const PLUGIN_SHORT_NAME = 'edem';
    const PLUGIN_VERSION = '2.6.0';
    const PLUGIN_DATE = '28.08.2021';

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
    const CHANNEL_LIST_URL = 'edem_channel_list.xml';
    const EPG_URL_FORMAT = 'http://epg.ott-play.com/edem/epg/%d.json';
    const TVG_URL_FORMAT = 'http://www.teleguide.info/kanal%d_%s.html';
    const EPG_PROVIDER = 'ott-play';
    const TVG_PROVIDER = 'tvguide.info';

    protected static $TVG_PARSER = 'HD::parse_epg_html';

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
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

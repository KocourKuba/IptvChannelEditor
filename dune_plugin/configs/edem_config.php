<?php
require_once 'lib/default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    public static $PLUGIN_NAME = 'iEdem/iLook TV';
    public static $PLUGIN_SHORT_NAME = 'edem';
    public static $PLUGIN_VERSION = '2.6.0';
    public static $PLUGIN_DATE = '01.09.2021';

    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
    public static $CHANNEL_LIST_URL = 'edem_channel_list.xml';
    public static $EPG1_URL_FORMAT = 'http://epg.ott-play.com/edem/epg/%s.json'; // epg_id
    public static $EPG2_URL_FORMAT = 'http://www.teleguide.info/kanal%s_%s.html'; // epg_id date(YYYYMMDD)

    protected static $TVG_PARSER = 'html';

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

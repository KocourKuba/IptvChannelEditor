<?php
require_once 'lib/default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const PLUGIN_NAME = 'Sharaclub TV';
    const PLUGIN_SHORT_NAME = 'sharaclub';
    const PLUGIN_VERSION = '1.0.0';
    const PLUGIN_DATE = '28.08.2021';

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    const CHANNEL_LIST_URL = 'sharaclub_channel_list.xml';
    const EPG_URL_FORMAT = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s';
    const TVG_URL_FORMAT = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s';
    //const EPG_URL_FORMAT = 'https://list.playtv.pro/f/epg_lite.xml.gz';
    const EPG_PROVIDER = 'sharaclub';

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        if ($archive_ts > 0) {
            $url = str_replace("index.m3u8", "archive-" . $archive_ts . "-10800.m3u8", $url);
        }

        hd_print("AdjustStreamUri: $url");
        return $url;
    }
}

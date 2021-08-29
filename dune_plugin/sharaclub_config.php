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
    const EPG_URL_FORMAT = 'https://list.playtv.pro/f/epg.xml.gz';
    const EPG_PROVIDER = 'sharaclub';

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        if ($archive_ts > 0) {
            $url = str_replace("index.m3u8", "archive-" . $archive_ts . "-10800.m3u8", $url);
        }

        hd_print("AdjustStreamUri: $url");
        return $url;
    }

    public final function GetEPG(IChannel $channel, $day_start_ts)
    {
        $epg = array();
        // epg same as channel id
        $epg_id = $channel->get_channel_id();
        // if epg is empty, no need to fetch data
        if (empty($epg_id)) {
            hd_print("EPG not defined for channel '" . $channel->get_title() . "'");
            return $epg;
        }

        $provider = $this->GET_EPG_PROVIDER();
        try {
            // xml epg source, no backup source
            hd_print("Fetching EPG ID from primary epg source '$provider': '$epg_id'");
            $epg = HD::parse_epg_xml($this->GET_EPG_URL_FORMAT(), $epg_id, $day_start_ts, $this->get_epg_cache_dir());
        } catch (Exception $ex) {
            hd_print("Can't fetch EPG ID from primary epg source '$provider':" . $ex->getMessage());
            return $epg;
        }

        $this->SortAndStore($channel, $day_start_ts, $epg);

        return $epg;
    }
}

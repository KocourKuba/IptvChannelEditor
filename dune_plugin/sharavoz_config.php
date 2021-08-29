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
    const EPG_URL_FORMAT = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s';
    const TVG_URL_FORMAT = 'http://api.program.spr24.net/api/program?epg=%s&date=%s';
    const EPG_PROVIDER = 'arlekino';
    const TVG_PROVIDER = 'sharavoz';

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
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

    public final function GetEPG(IChannel $channel, $day_start_ts)
    {
        $epg = array();
        $epg_id = intval($channel->get_epg_id());
        $tvg_id = intval($channel->get_tvg_id());

        // if all tvg & epg are empty, no need to fetch data
        if ($tvg_id === 0 && $epg_id === 0) {
            hd_print("EPG not defined for channel '" . $channel->get_title() . "'");
            return $epg;
        }

        $epg_date = gmdate("Ymd", $day_start_ts);
        $provider = $this->GET_EPG_PROVIDER();
        try {
            // arlekino - primary epg source
            hd_print("Fetching EPG ID from primary epg source '$provider': '$epg_id'");
            $epg = HD::parse_epg_json(sprintf($this->GET_EPG_URL_FORMAT(), $epg_id, $epg_date), $day_start_ts);
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source '$provider'");
                // sharavoz - backup epg source
                $provider = $this->GET_TVG_PROVIDER();
                hd_print("Fetching EPG ID from secondary epg source '$provider': '$tvg_id' DATE: $epg_date");
                $epg = HD::parse_epg_json(sprintf($this->GET_TVG_URL_FORMAT(), $tvg_id, $epg_date), $day_start_ts);
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from '$provider': " . $ex->getMessage());
                return $epg;
            }
        }

        $this->SortAndStore($channel, $day_start_ts, $epg);

        return $epg;
    }
}

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

    public final function GetEPG(IChannel $channel, $day_start_ts)
    {
        $epg_id = intval($channel->get_epg_id());
        $tvg_id = intval($channel->get_tvg_id());

        // if all tvg & epg are empty, no need to fetch data
        $epg = array();
        if ($tvg_id === 0 && $epg_id === 0) {
            hd_print("EPG not defined for channel '" . $channel->get_title() . "'");
            return $epg;
        }

        $epg_date = gmdate("Ymd", $day_start_ts);
        $provider = $this->GET_EPG_PROVIDER();
        try {
            // ott-play - primary epg source
            hd_print("Fetching EPG ID from primary epg source '$provider': '$epg_id'");
            $url = sprintf($this->GET_EPG_URL_FORMAT(), $epg_id);
            $epg = HD::parse_epg_json($url, $day_start_ts);
        } catch (Exception $ex) {
            try {
                hd_print("Can't fetch EPG ID from primary epg source '$provider': " . $ex->getMessage());
                // tvguide.info - backup epg source
                $provider = $this->GET_TVG_PROVIDER();
                hd_print("Fetching EPG ID from secondary epg source '$provider': '$tvg_id' DATE: $epg_date");
                $epg = EdemPluginConfig::parse_epg_html(sprintf($this->GET_TVG_URL_FORMAT(), $tvg_id, $epg_date), $epg_date);
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from '$provider': " . $ex->getMessage());
                return $epg;
            }
        }

        $this->SortAndStore($channel, $day_start_ts, $epg);

        return $epg;
    }

    /**
     * @param $url
     * @param $epg_date
     * @return array
     */
    protected static function parse_epg_html($url, $epg_date)
    {
        // html parse for tvguide.info
        // tvguide.info time in GMT+3 (moscow time)

        $epg = array();
        $e_time = strtotime("$epg_date, 0300 GMT+3");

        try {
            hd_print($url);
            $doc = HD::http_get_document($url);
        }
        catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        preg_match_all('|<div id="programm_text">(.*?)</div>|', $doc, $keywords);
        foreach ($keywords[1] as $qid) {
            $qq = strip_tags($qid);
            preg_match_all('|(\d\d:\d\d)&nbsp;(.*?)&nbsp;(.*)|', $qq, $keyw);
            $time = $keyw[1][0];
            $u_time = strtotime("$epg_date $time GMT+3");
            $last_time = ($u_time < $e_time) ? $u_time + 86400 : $u_time;
            $epg[$last_time]["title"] = HD::unescape_entity_string($keyw[2][0]);
            $epg[$last_time]["desc"] = HD::unescape_entity_string($keyw[3][0]);
        }

        return $epg;
    }
}

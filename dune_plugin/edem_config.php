<?php
require_once 'lib/default_config.php';
require_once 'starnet_channel.php';

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
        $this->TVG_PROVIDER = 'tvguide.info';
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

    public function GetEPG($channel, $day_start_ts)
    {
        $epg_id = intval($channel->get_epg_id());
        $tvg_id = intval($channel->get_tvg_id());

        $this->create_cache_folder();
        $cache_file = $this->EPG_CACHE_DIR . $this->EPG_CACHE_FILE . $channel->get_id() . "_" . $day_start_ts;

        // if all tvg & epg are empty, no need to fetch data
        $epg = array();
        if (DefaultConfig::LoadCachedEPG($cache_file, $epg) === false
            && ($tvg_id !== 0 || $epg_id !== 0)) {

            $epg_date = gmdate("Ymd", $day_start_ts);
            try {
                // tvguide used as backup of ott-play epg source
                hd_print("Fetching EPG ID from primary epg source '$this->EPG_PROVIDER': $epg_id DATE: $epg_date");
                $url = sprintf($this->EPG_URL_FORMAT, $epg_id);
                $epg = HD::parse_epg_json($url, $day_start_ts);
            } catch (Exception $ex) {
                try {
                    hd_print("Can't fetch EPG ID from primary epg source '$this->EPG_PROVIDER': " . $ex->getMessage());
                    hd_print("Fetching EPG ID from secondary epg source '$this->TVG_PROVIDER': $tvg_id DATE: $epg_date");
                    $epg = $this->parse_epg_html(sprintf($this->TVG_URL_FORMAT, $tvg_id, $epg_date), $epg_date);
                } catch (Exception $ex) {
                    hd_print("Can't fetch EPG ID from '$this->TVG_PROVIDER': " . $ex->getMessage());
                }
            }
        }

        // sort epg by date
        if (count($epg) > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents($cache_file, serialize($epg));
        }

        return $epg;
    }

    /**
     * @param $url
     * @param $epg_date
     * @return array
     */
    protected function parse_epg_html($url, $epg_date)
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

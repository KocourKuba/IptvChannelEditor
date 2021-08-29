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

    public function GetEPG($channel, $day_start_ts)
    {
        $this->create_cache_folder();
        $cache_file = $this->EPG_CACHE_DIR . $this->EPG_CACHE_FILE . $channel->get_id() . "_" . $day_start_ts;

        $epg = array();
        $epg_id = intval($channel->get_epg_id());
        // if epg is empty, no need to fetch data
        if (DefaultConfig::LoadCachedEPG($cache_file, $epg) === false
            && $epg_id !== 0) {
            try {
                // xml epg source, no backup source
                hd_print("Fetching EPG ID from primary epg source '$this->EPG_PROVIDER': $epg_id");
                $epg = HD::parse_epg_xml($this->EPG_URL_FORMAT, $epg_id, $day_start_ts, $this->EPG_CACHE_DIR);
            } catch (Exception $ex) {
                hd_print("Can't fetch EPG ID from primary epg source '$this->EPG_PROVIDER':" . $ex->getMessage());
            }
        }

        // sort epg by date
        if (count($epg) > 0) {
            ksort($epg, SORT_NUMERIC);
            file_put_contents($cache_file, serialize($epg));
        }

        return $epg;
    }
}

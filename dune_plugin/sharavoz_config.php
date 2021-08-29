<?php
require_once 'lib/default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        $this->PluginName = 'Sharavoz TV';
        $this->PluginVersion = '1.0.0';
        $this->PluginDate = '28.08.2021';

        $this->BG_PICTURE = 'plugin_file://bg_edem.jpg';
        $this->MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
        $this->CHANNEL_LIST_URL = 'sharavoz_channel_list.xml';
        $this->EPG_URL_FORMAT = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s';
        $this->TVG_URL_FORMAT = 'http://api.program.spr24.net/api/program?epg=%s&date=%s';
        $this->EPG_CACHE_DIR = '/tmp/sharavoz_epg/';
        $this->TVG_PROVIDER = 'sharavoz';
        $this->EPG_PROVIDER = 'arlekino';
    }

    public function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
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

    public function GetEPG($channel, $day_start_ts)
    {
        $this->create_cache_folder();
        $cache_file = $this->EPG_CACHE_DIR . $this->EPG_CACHE_FILE . $channel->get_id() . "_" . $day_start_ts;

        $epg_id = intval($channel->get_epg_id());
        $tvg_id = intval($channel->get_tvg_id());

        // if all tvg & epg are empty, no need to fetch data
        $epg = array();
        if (DefaultConfig::LoadCachedEPG($cache_file, $epg) === false
            && ($tvg_id !== 0 || $epg_id !== 0)) {

            $epg_date = gmdate("Ymd", $day_start_ts);
            try {
                // sharavoz used as backup of arlekino epg source
                hd_print("Fetching EPG ID from primary epg source '$this->EPG_PROVIDER': $epg_id DATE: $epg_date");
                $epg = HD::parse_epg_json(sprintf($this->EPG_URL_FORMAT, $epg_id, $epg_date), $day_start_ts);
            } catch (Exception $ex) {
                try {
                    hd_print("Can't fetch EPG ID from primary epg source '$this->EPG_PROVIDER'");
                    hd_print("Fetching EPG ID from secondary epg source '$this->TVG_PROVIDER': $tvg_id DATE: $epg_date");
                    $epg = HD::parse_epg_json(sprintf($this->TVG_URL_FORMAT, $tvg_id, $epg_date), $day_start_ts);
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
}

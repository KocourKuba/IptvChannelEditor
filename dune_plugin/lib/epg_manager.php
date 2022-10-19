<?php
require_once 'hd.php';
require_once 'epg_xml_parser.php';

class Epg_Manager
{
    /**
     * @var default_config
     */
    public $config;

    /**
     * @param default_config $config
     */
    public function __construct(default_config $config)
    {
        $this->config = $config;
    }

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param Channel $channel
     * @param string $type
     * @param int $day_start_ts
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_epg(Channel $channel, $type, $day_start_ts, $plugin_cookies)
    {
        $params = $this->config->get_epg_params($type);

        if (empty($params[EPG_URL])) {
            hd_print("$type EPG url not defined");
            throw new Exception("$type EPG url not defined");
        }

        switch ($type) {
            case EPG_FIRST:
                $epg_id = $channel->get_epg_id();
                break;
            case EPG_SECOND:
                $epg_id = $channel->get_tvg_id();
                break;
            default:
                $epg_id = '';
        }

        if (empty($epg_id) && strpos($params[EPG_URL], '{ID}') === false) {
            hd_print("EPG: $epg_id not defined");
            throw new Exception("EPG: $epg_id not defined");
        }

        $epg_id = str_replace(' ', '%20', $epg_id);

        hd_print("Fetching EPG for ID: '$epg_id'");
        $epg_url = str_replace(array('{EPG_ID}', '{ID}'), array($epg_id, $channel->get_id()), $params[EPG_URL]);
        if (strpos($epg_url, '{DATE}') !== false) {
            $date_format = str_replace(
                array('{YEAR}', '{MONTH}', '{DAY}'),
                array('Y', 'm', 'd'),
                $params[EPG_DATE_FORMAT]);
            $epg_date = gmdate($date_format, $day_start_ts + get_local_time_zone_offset());
            $epg_url = str_replace('{DATE}', $epg_date, $epg_url);
            hd_print("From DATE: $epg_date");
        }

        if (strpos($epg_url, '{TIMESTAMP}') !== false) {
            $epg_url = str_replace('{TIMESTAMP}', $day_start_ts, $epg_url);
            hd_print("From Timestamp: $day_start_ts");
        }

        if (strpos($epg_url, '{TOKEN}') !== false) {
            $epg_url = str_replace('{TOKEN}', isset($plugin_cookies->token) ? $plugin_cookies->token : '', $epg_url);
        }

        $cache_dir = get_temp_path("epg/");
        $cache_file = $cache_dir . "epg_channel_" . hash('crc32', $epg_url);

        $from_cache = false;
        $out_epg = array();
        $epg = array();
        if (file_exists($cache_file)) {
            $now = time();
            $cache_expired = filemtime($cache_file) + 60 * 60 * 24;
            hd_print("Cache expired at $cache_expired now $now");
            if ($cache_expired > time()) {
                $epg = unserialize(file_get_contents($cache_file));
                $from_cache = (count($epg) !== 0);
                hd_print("Loading EPG entries from cache: $cache_file - " . ($from_cache ? "success" : "failed"));
            }
        }

        if ($from_cache === false && $params[EPG_PARSER] === 'json') {
            $epg = self::get_epg_json($epg_url, $params);
        }

        $counts = count($epg);
        hd_print("Total entries: $counts");
        if ($counts !== 0) {
            // entries present
            if (!is_dir($cache_dir) && !(mkdir($cache_dir) && is_dir($cache_dir))) {
                hd_print("Directory '$cache_dir' was not created");
            } else {
                // if not in cache save downloaded data
                if ($from_cache === false) {
                    // save downloaded epg
                    self::save_cache($epg, $cache_file);
                }

                // filter out epg only for selected day
                $day_end_ts = $day_start_ts + 86400;

                $date_start_l = format_datetime("Y-m-d H:i", $day_start_ts);
                $date_end_l = format_datetime("Y-m-d H:i", $day_end_ts);
                hd_print("Fetch entries for localtime from: $date_start_l to: $date_end_l");

                $date_start_gm = gmdate('Y-m-d H:i', $day_start_ts);
                $date_end_gm = gmdate('Y-m-d H:i', $day_end_ts);
                hd_print("Fetch entries as UTC from: $day_start_ts ($date_start_gm) to: $day_end_ts ($date_end_gm)");
                foreach ($epg as $time_start => $entry) {
                    if ($time_start >= $day_start_ts && $time_start < $day_end_ts) {
                        $out_epg[$time_start] = $entry;
                        //hd_print("$time_start (" . gmdate('Y-m-d H:i', $time_start) . " / " . format_datetime('Y-m-d H:i', $time_start) . "): " . $entry[EPG_NAME]);
                    }
                }
            }
        }

        return $out_epg;
    }

    protected static function save_cache($day_epg, $cache_file) {
        ksort($day_epg, SORT_NUMERIC);
        file_put_contents($cache_file, serialize($day_epg));
    }

    /**
     * request server for epg and parse json response
     * @param string $url
     * @param array $parser_params
     * @return array
     * @throws Exception
     */
    protected static function get_epg_json($url, $parser_params)
    {
        $day_epg = array();

        try {
            $ch_data = HD::DownloadJson($url);
            if (empty($ch_data)) {
                hd_print("Empty document returned.");
                return $day_epg;
            }
        } catch (Exception $ex) {
            hd_print("http exception: " . $ex->getMessage());
            return $day_epg;
        }

        if (!empty($parser_params[EPG_ROOT])) {
            foreach (explode('|', $parser_params[EPG_ROOT]) as $level) {
                $epg_root = trim($level, "[]");
                $ch_data = $ch_data[$epg_root];
            }
        }
        // hd_print("json epg root: " . $parser_params[EPG_ROOT]);
        // hd_print("json start: " . $parser_params[EPG_START]);
        // hd_print("json end: " . $parser_params[EPG_END]);
        // hd_print("json title: " . $parser_params[EPG_NAME]);
        // hd_print("json desc: " . $parser_params[EPG_DESC]);

        // collect all program that starts after day start and before day end
        $prev_start = 0;
        $no_end = empty($parser_params[EPG_END]);
        $no_timestamp = !empty($parser_params[EPG_TIME_FORMAT]);
        $use_duration = $parser_params[EPG_USE_DURATION];
        foreach ($ch_data as $entry) {
            $program_start = $entry[$parser_params[EPG_START]];
            //hd_print("epg_start $program_start");

            if ($no_timestamp) {
                // start time not the timestamp
                // parsed time assumed as UTC+00
                // 'd-m-Y H:i'
                $time_format = str_replace(
                    array('{YEAR}', '{MONTH}', '{DAY}', '{HOUR}', '{MIN}'),
                    array('Y', 'm', 'd', 'H', 'i'),
                    $parser_params[EPG_TIME_FORMAT]);
                $dt = DateTime::createFromFormat($time_format, $program_start, new DateTimeZone('UTC'));
                $program_start = $dt->getTimestamp() - $parser_params[EPG_TIMEZONE] * 3600; // subtract real EPG timezone
            }

            // prefill data to avoid undefined index notice
            $day_epg[$program_start][EPG_END] = 0;
            $day_epg[$program_start][EPG_NAME] = '';
            $day_epg[$program_start][EPG_DESC] = '';

            if ($use_duration) {
                $day_epg[$program_start][EPG_END] = $program_start + (int)$entry[$parser_params[EPG_END]];
            } else if ($no_end) {
                if ($prev_start !== 0) {
                    $day_epg[$prev_start][EPG_END] = $program_start;
                }
                $prev_start = $program_start;
            } else {
                $day_epg[$program_start][EPG_END] = (int)$entry[$parser_params[EPG_END]];
            }

            if (isset($entry[$parser_params[EPG_NAME]])) {
                $day_epg[$program_start][EPG_NAME] = HD::unescape_entity_string($entry[$parser_params[EPG_NAME]]);
            }
            if (isset($entry[$parser_params[EPG_DESC]])) {
                $day_epg[$program_start][EPG_DESC] = HD::unescape_entity_string($entry[$parser_params[EPG_DESC]]);
            }
        }

        if ($no_end && $prev_start !== 0) {
            $day_epg[$prev_start][EPG_END] = $prev_start + 3600; // fake end
        }

        ksort($day_epg);
        return $day_epg;
    }

    /**
     * request server for XMLTV epg and parse xml or xml.gx response
     * @param string $url
     * @param int $day_start_ts
     * @param string $epg_id
     * @param string $cache_dir
     * @return array
     */
    protected static function get_epg_xml($url, $day_start_ts, $epg_id, $cache_dir)
    {
        $epg = array();
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);

        try {
            // checks if epg already loaded
            preg_match('/^.*\/(.+)$/', $url, $match);
            $epgCacheFile = sprintf("%s/%s_%s", $cache_dir, $match[1], $day_start_ts);
            if (!file_exists($epgCacheFile)) {
                hd_print("epg uri: $url");
                $doc = HD::http_get_document($url);
                if (!file_put_contents($epgCacheFile, $doc)) {
                    hd_print("Writing to $epgCacheFile is not possible!");
                }
            }

            // parse
            $Parser = new Epg_Xml_Parser();
            $Parser->setFile($epgCacheFile);
            $Parser->setChannelfilter($epg_id);
            $Parser->parseEpg();
            $epg_data = $Parser->getEpgData();
            if (empty($epg_data)) {
                hd_print("No EPG data found");
            } else {
                foreach ($epg_data as $channel) {
                    if ($channel->time >= $epg_date_start && $channel->time < $epg_date_end) {
                        $epg[$channel->time][EPG_NAME] = HD::unescape_entity_string($channel->name);
                        $epg[$channel->time][EPG_DESC] = HD::unescape_entity_string($channel->descr);
                    }
                }
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        return $epg;
    }
}
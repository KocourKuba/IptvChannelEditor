<?php
require_once 'hd.php';

class Epg_Manager
{
    /**
     * @var default_config
     */
    public $config;

    /**
     * @var string
     */
    protected $cache_dir;

    /**
     * @var string
     */
    protected $dune_ip;

    /**
     * @param default_config $config
     */
    public function __construct(default_config $config)
    {
        $this->config = $config;
        $this->cache_dir = get_temp_path("epg");
        $this->dune_ip = get_ip_address();
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

        if (empty($params[Epg_Params::EPG_URL])) {
            hd_print("$type EPG url not defined");
            throw new Exception("$type EPG url not defined");
        }

        if (!is_dir($this->cache_dir) && !(mkdir($this->cache_dir) && is_dir($this->cache_dir))) {
            hd_print("Unable to create directory '$this->cache_dir'!!!");
            throw new Exception("Unable to create directory '$this->cache_dir'!!!");
        }

        switch ($type) {
            case Plugin_Constants::EPG_FIRST:
                $epg_id = $channel->get_epg_id();
                break;
            case Plugin_Constants::EPG_SECOND:
                $epg_id = $channel->get_tvg_id();
                break;
            default:
                $epg_id = '';
        }

        $channel_id = $channel->get_id();
        $channel_title = $channel->get_title();
        if (empty($epg_id) && strpos($params[Epg_Params::EPG_URL], '{ID}') === false) {
            hd_print("EPG ID: $epg_id not defined for channel $channel_id ($channel_title)");
            throw new Exception("EPG: $epg_id not defined");
        }

        hd_print("Try to load EPG ID: '$epg_id' for channel '$channel_id' ($channel_title)");
        $epg_id = str_replace(' ', '%20', $epg_id);
        $epg_url = str_replace(array('{EPG_ID}', '{ID}', '{DUNE_IP}'), array($epg_id, $channel_id, $this->dune_ip), $params[Epg_Params::EPG_URL]);
        if (strpos($epg_url, '{DATE}') !== false) {
            $date_format = str_replace(
                array('{YEAR}', '{MONTH}', '{DAY}'),
                array('Y', 'm', 'd'),
                $params[Epg_Params::EPG_DATE_FORMAT]);
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

        $epg_url = str_replace('#', '%23', $epg_url);
        $hash = hash('crc32', $epg_url);
        $epg_cache_file = "$this->cache_dir/epg_channel_$hash";
        $day_epg_cache = $epg_cache_file . "_$day_start_ts";
        if (file_exists($day_epg_cache)) {
            hd_print("Loading day entries for EPG ID: '$epg_id' from cache: $day_epg_cache");
            return self::load_cache($day_epg_cache);
        }

        $from_cache = false;
        $program_epg = array();
        if (file_exists($epg_cache_file)) {
            $now = time();
            $cache_expired = filemtime($epg_cache_file) + 60 * 60 * 4;
            if ($cache_expired > time()) {
                $program_epg = self::load_cache($epg_cache_file);
                $from_cache = true;
                hd_print("Loading all entries for EPG ID: '$epg_id' from cache: $epg_cache_file");
            } else {
                hd_print("Cache expired at $cache_expired now $now");
            }
        }

        if ($from_cache === false && $params[Plugin_Constants::EPG_PARSER] === 'json') {
            hd_print("Fetching EPG ID: '$epg_id' from server");
            $program_epg = self::get_epg_json($epg_url, $params);
            // save downloaded data
        }

        $counts = count($program_epg);
        if ($counts !== 0 && $from_cache === false) {
            self::save_cache($program_epg, $epg_cache_file);
        }

        //hd_print("Total entries: $counts");
        if ($counts === 0) {
            return array();
        }

        // filter out epg only for selected day
        $day_end_ts = $day_start_ts + 86400;

        $date_start_l = format_datetime("Y-m-d H:i", $day_start_ts);
        $date_end_l = format_datetime("Y-m-d H:i", $day_end_ts);
        hd_print("Fetch entries for localtime from: $date_start_l to: $date_end_l");

        $date_start_gm = gmdate('Y-m-d H:i', $day_start_ts);
        $date_end_gm = gmdate('Y-m-d H:i', $day_end_ts);
        hd_print("Fetch entries as UTC from: $day_start_ts ($date_start_gm) to: $day_end_ts ($date_end_gm)");

        $day_epg = array();
        foreach ($program_epg as $time_start => $entry) {
            if ($time_start >= $day_start_ts && $time_start < $day_end_ts) {
                $day_epg[$time_start] = $entry;
                //hd_print("$time_start (" . gmdate('Y-m-d H:i', $time_start) . " / " . format_datetime('Y-m-d H:i', $time_start) . "): " . $entry[EPG_NAME]);
            }
        }

        if (!empty($day_epg)) {
            self::save_cache($day_epg, $day_epg_cache);
            hd_print("Day entries for EPG ID: '$epg_id' saved to cache: $day_epg_cache");
        }

        return $day_epg;
    }

    /**
     * @param array $day_epg
     * @param string $cache_file
     */
    protected static function save_cache($day_epg, $cache_file)
    {
        ksort($day_epg, SORT_NUMERIC);
        file_put_contents($cache_file, serialize($day_epg));
    }

    /**
     * @param string $cache_file
     * @return array
     */
    protected static function load_cache($cache_file)
    {
        return unserialize(file_get_contents($cache_file));
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

        if (!empty($parser_params[Epg_Params::EPG_ROOT])) {
            foreach (explode('|', $parser_params[Epg_Params::EPG_ROOT]) as $level) {
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
        $no_end = empty($parser_params[Epg_Params::EPG_END]);
        $no_timestamp = !empty($parser_params[Epg_Params::EPG_TIME_FORMAT]);
        $use_duration = $parser_params[Epg_Params::EPG_USE_DURATION];
        foreach ($ch_data as $entry) {
            $program_start = $entry[$parser_params[Epg_Params::EPG_START]];
            //hd_print("epg_start $program_start");

            if ($no_timestamp) {
                // start time not the timestamp
                // parsed time assumed as UTC+00
                // 'd-m-Y H:i'
                $time_format = str_replace(
                    array('{YEAR}', '{MONTH}', '{DAY}', '{HOUR}', '{MIN}'),
                    array('Y', 'm', 'd', 'H', 'i'),
                    $parser_params[Epg_Params::EPG_TIME_FORMAT]);
                $dt = DateTime::createFromFormat($time_format, $program_start, new DateTimeZone('UTC'));
                $program_start = $dt->getTimestamp() - $parser_params[Epg_Params::EPG_TIMEZONE] * 3600; // subtract real EPG timezone
            }

            // prefill data to avoid undefined index notice
            $day_epg[$program_start][Epg_Params::EPG_END] = 0;
            $day_epg[$program_start][Epg_Params::EPG_NAME] = '';
            $day_epg[$program_start][Epg_Params::EPG_DESC] = '';

            if ($use_duration) {
                $day_epg[$program_start][Epg_Params::EPG_END] = $program_start + (int)$entry[$parser_params[Epg_Params::EPG_END]];
            } else if ($no_end) {
                if ($prev_start !== 0) {
                    $day_epg[$prev_start][Epg_Params::EPG_END] = $program_start;
                }
                $prev_start = $program_start;
            } else {
                $day_epg[$program_start][Epg_Params::EPG_END] = (int)$entry[$parser_params[Epg_Params::EPG_END]];
            }

            if (isset($entry[$parser_params[Epg_Params::EPG_NAME]])) {
                $day_epg[$program_start][Epg_Params::EPG_NAME] = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_NAME]]);
            }
            if (isset($entry[$parser_params[Epg_Params::EPG_DESC]])) {
                $desc = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_DESC]]);
                $desc = str_replace('<br>', PHP_EOL, $desc);
                $day_epg[$program_start][Epg_Params::EPG_DESC] = $desc;
            }
        }

        if ($no_end && $prev_start !== 0) {
            $day_epg[$prev_start][Epg_Params::EPG_END] = $prev_start + 3600; // fake end
        }

        ksort($day_epg);
        return $day_epg;
    }
}
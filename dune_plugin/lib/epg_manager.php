<?php
require_once 'hd.php';
require_once 'v32.php';
require_once 'epg_xml_parser.php';

class Epg_Manager
{
    /**
     * @var Default_Config
     */
    public $config;

    /**
     * @param Default_Config $config
     */
    public function __construct(Default_Config $config)
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

        if (empty($params['epg_url'])) {
            hd_print("$type EPG url not defined");
            throw new Exception("$type EPG url not defined");
        }

        switch ($type) {
            case 'first':
                $epg_id = $channel->get_epg_id();
                break;
            case 'second':
                $epg_id = $channel->get_tvg_id();
                break;
            default:
                $epg_id = '';
        }

        if (empty($epg_id)) {
            hd_print("EPG: $epg_id not defined");
            throw new Exception("EPG: $epg_id not defined");
        }

        if ($params['epg_use_mapper']) {
            $mapper = $params['epg_id_mapper'];
            if (empty($mapper)) {
                $mapper = HD::MapTvgID($params['epg_mapper_url']);
                hd_print("TVG ID Mapped: " . count($mapper));
                $this->config->set_epg_param($type, 'epg_id_mapper', $mapper);
            }

            if (array_key_exists($epg_id, $mapper)) {
                hd_print("EPG id replaced: $epg_id -> " . $mapper[$epg_id]);
                $epg_id = $mapper[$epg_id];
            }
        } else if ($params['epg_use_hash']) {
            $xx_hash = new V32();
            $epg_id = $xx_hash->hash($epg_id);
        } else {
            $epg_id = str_replace(' ', '%20', $epg_id);
        }

        $epg_date = gmdate($params['epg_date_format'], $day_start_ts);
        $epg_url = str_replace(
            array('{TOKEN}', '{CHANNEL}', '{DATE}', '{TIME}'),
            array(isset($plugin_cookies->token) ? $plugin_cookies->token : '', $epg_id, $epg_date, $day_start_ts),
            $params['epg_url']);

        hd_print("Fetching EPG for ID: '$epg_id' DATE: $epg_date");

        $cache_dir = DuneSystem::$properties['tmp_dir_path'] . "/epg";
        $cache_file = sprintf("%s/epg_channel_%s", $cache_dir, hash('crc32', $epg_url));

        $from_cache = false;
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

        if ($from_cache === false && $params['epg_parser'] === 'json') {
            $epg = self::get_epg_json($epg_url, $params);
        }

        $out_epg = array();
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
                $epg_date_start = strtotime('-1 hour', $day_start_ts);
                $epg_date_end = strtotime('+1 day', $day_start_ts);
                hd_print("Fetch entries from: $epg_date_start to: $epg_date_end");
                foreach ($epg as $time_start => $entry) {
                    if ($epg_date_start <= $time_start && $time_start <= $epg_date_end) {
                        $out_epg[$time_start] = $entry;
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
     */
    protected static function get_epg_json($url, $parser_params)
    {
        $day_epg = array();

        try {
            $doc = HD::http_get_document($url);
            if (empty($doc)) {
                hd_print("Empty document returned.");
                return $day_epg;
            }
        } catch (Exception $ex) {
            hd_print("http exception: " . $ex->getMessage());
            return $day_epg;
        }

        // stripe UTF8 BOM if exists
        $ch_data = json_decode(ltrim($doc, "\xEF\xBB\xBF"), true);
        if (!empty($parser_params['epg_root'])) {
            foreach (explode('|', $parser_params['epg_root']) as $level) {
                $epg_root = $level;
                $ch_data = $ch_data[$epg_root];
            }
        }
        // hd_print("json epg root: " . $parser_params['epg_root']);
        // hd_print("json start: " . $parser_params['epg_start']);
        // hd_print("json end: " . $parser_params['epg_end']);
        // hd_print("json title: " . $parser_params['epg_title']);
        // hd_print("json desc: " . $parser_params['epg_desc']);

        // collect all program that starts after day start and before day end
        $prev_start = 0;
        $no_end = empty($parser_params['epg_end']);
        $no_timestamp = !empty($parser_params['epg_time_format']);
        $use_duration = $parser_params['epg_use_duration'];
        foreach ($ch_data as $entry) {
            $program_start = $entry[$parser_params['epg_start']];
            hd_print("epg_start $program_start");

            if ($no_timestamp) {
                // start time not the timestamp
                $dt = DateTime::createFromFormat($parser_params['epg_time_format'], $program_start, new DateTimeZone($parser_params['epg_timezone']));
                $program_start = $dt->getTimestamp();
                if (is_need_daylight_fix())
                    $program_start += 3600;

                hd_print("epg_start: $program_start");
            }

            if ($use_duration) {
                $day_epg[$program_start]['epg_end'] = $program_start + (int)$entry[$parser_params['epg_end']];
            } else if ($no_end) {
                if ($prev_start !== 0) {
                    $day_epg[$prev_start]['epg_end'] = $program_start;
                }
                $prev_start = $program_start;
            } else {
                $day_epg[$program_start]['epg_end'] = (int)$entry[$parser_params['epg_end']];
            }

            $day_epg[$program_start]['epg_title'] = HD::unescape_entity_string($entry[$parser_params['epg_title']]);
            $day_epg[$program_start]['epg_desc'] = HD::unescape_entity_string($entry[$parser_params['epg_desc']]);
        }

        if ($no_end && $prev_start !== 0) {
            $day_epg[$prev_start]['epg_end'] = $prev_start + 60 * 60; // fake end
        }
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
                        $epg[$channel->time]['epg_title'] = HD::unescape_entity_string($channel->name);
                        $epg[$channel->time]['epg_desc'] = HD::unescape_entity_string($channel->descr);
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
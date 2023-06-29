<?php
require_once 'hd.php';

class Epg_Manager
{
    const EPG_CACHE_PATH = 'epg/';
    const EPG_XMLTV_CACHE_PATH = 'xml_epg/';

    /**
     * @var default_config
     */
    public $config;

    /**
     * contains current dune IP
     * @var string
     */
    protected $dune_ip;

    /**
     * contains memory epg cache
     * @var array
     */
    protected $epg_cache = array();

    /**
     * contains all urls to download XMLTV EPG
     * @var array
     */
    protected $xmltv_urls;

    /**
     * contains filename of the cached xmltv file (if original is packed points to unpacked version)
     * @var array
     */
    protected $xml_cached_file;

    /**
     * contains parsed epg for current xmltv file (ALL data!)
     * @var array
     */
    public $xmltv_data;

    /**
     * @param default_config $config
     */
    public function __construct(default_config $config)
    {
        $this->config = $config;
        $cache_dir = get_temp_path(self::EPG_CACHE_PATH);
        $xcache_dir = get_temp_path(self::EPG_XMLTV_CACHE_PATH);
        if (!is_dir($cache_dir) && !(mkdir($cache_dir) && is_dir($cache_dir))) {
            hd_print("Unable to create directory '$cache_dir'!!!");
        }

        if (!is_dir($xcache_dir) && !(mkdir($xcache_dir) && is_dir($xcache_dir))) {
            hd_print("Unable to create directory '$xcache_dir'!!!");
        }
    }

    public function clear_xmltv_urls()
    {
        $this->xmltv_urls = array();
        $this->xml_cached_file = array();
    }

    /**
     * @param $idx string
     * @param $xmltv_url string
     */
    public function set_xmltv_url($idx, $xmltv_url)
    {
        $this->xmltv_urls[$idx] = $xmltv_url;
        $this->xml_cached_file[$idx] = basename($xmltv_url);
        if (preg_match('|\.gz|', $this->xml_cached_file[$idx])) {
            $this->xml_cached_file[$idx] = substr($this->xml_cached_file[$idx], 0, -3);
        }

        hd_print(__METHOD__ . ": index: $idx, set cached file: {$this->xml_cached_file[$idx]} for url {$this->xmltv_urls[$idx]}");
    }

    /**
     * @param $idx string
     * @return string
     */
    public function get_xmltv_url($idx)
    {
        return (isset($this->xmltv_urls[$idx])) ? $this->xmltv_urls[$idx] : '';
    }

    /**
     * @return array
     */
    public function get_xmltv_urls()
    {
        return $this->xmltv_urls;
    }

    /**
     * @param $idx string
     * @return string
     */
    public function get_xml_cached_file($idx)
    {
        return isset($this->xml_cached_file[$idx]) ? get_temp_path(self::EPG_XMLTV_CACHE_PATH . $this->xml_cached_file[$idx]) : '';
    }

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param Channel $channel
     * @param int $day_start_ts
     * @param &$plugin_cookies
     * @return array|false
     */
    public function get_day_epg_items(Channel $channel, $day_start_ts, &$plugin_cookies)
    {
        try {
            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;
            $epg_id = $this->config->get_epg_id($channel, $epg_source);
            if (empty($epg_id)) {
                throw new Exception("EPG ID for $epg_source source not defined");
            }

            if (isset($this->epg_cache[$epg_id][$day_start_ts])) {
                hd_print(__METHOD__ . ": Load day EPG ID $epg_id ($day_start_ts) from memory cache ");
                return $this->epg_cache[$epg_id][$day_start_ts];
            }

            $params = $this->config->get_epg_params($epg_source);
            if (empty($params[Epg_Params::EPG_URL]) && $epg_source !== Plugin_Constants::EPG_INTERNAL) {
                $epg_source = ($epg_source === Plugin_Constants::EPG_FIRST) ? Plugin_Constants::EPG_SECOND : Plugin_Constants::EPG_FIRST;
                $params = $this->config->get_epg_params($epg_source);
                if (empty($params[Epg_Params::EPG_URL])) {
                    throw new Exception("$epg_source EPG url not defined");
                }
            }

            $channel_id = $channel->get_id();
            $channel_title = $channel->get_title();
            hd_print(__METHOD__ . ": Try to load EPG ID: '$epg_id' for channel '$channel_id' ($channel_title)");

            $epg_url = str_replace(
                array(Plugin_Macros::API_URL,
                    Plugin_Macros::EPG_DOMAIN,
                    Plugin_Macros::EPG_ID,
                    Plugin_Macros::ID,
                    Plugin_Macros::DUNE_IP
                ),
                array($this->config->get_feature(Plugin_Constants::PROVIDER_API_URL),
                    $params[Epg_Params::EPG_DOMAIN],
                    str_replace(' ', '%20', $epg_id),
                    $channel_id,
                    $this->dune_ip
                ),
                $params[Epg_Params::EPG_URL]);

            if (strpos($epg_url, Plugin_Macros::DATE) !== false) {
                $date_format = str_replace(
                    array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY),
                    array('Y', 'm', 'd'),
                    $params[Epg_Params::EPG_DATE_FORMAT]);

                $epg_date = gmdate($date_format, $day_start_ts + get_local_time_zone_offset());
                $epg_url = str_replace(Plugin_Macros::DATE, $epg_date, $epg_url);
                //hd_print(__METHOD__ . ": From DATE: $epg_date");
            }

            if (strpos($epg_url, Plugin_Macros::TIMESTAMP) !== false) {
                $epg_url = str_replace(Plugin_Macros::TIMESTAMP, $day_start_ts, $epg_url);
                //hd_print(__METHOD__ . ": From Timestamp: $day_start_ts");
            }

            if (strpos($epg_url, Plugin_Macros::TOKEN) !== false) {
                $epg_url = str_replace(Plugin_Macros::TOKEN, isset($plugin_cookies->token) ? $plugin_cookies->token : '', $epg_url);
            }

            $epg_url = str_replace('#', '%23', $epg_url);
            $hash = hash('crc32', $epg_url);
            $epg_cache_file = get_temp_path("epg/epg_channel_$hash");
            $day_epg_cache = $epg_cache_file . "_$day_start_ts";
            if (file_exists($day_epg_cache)) {
                //hd_print(__METHOD__ . ": Loading day entries for EPG ID: '$epg_id' from file cache: $day_epg_cache");
                return self::load_cache($day_epg_cache);
            }

            $from_cache = false;
            $program_epg = array();
            if (file_exists($epg_cache_file)) {
                $now = time();
                $max_check_time = 3600 * 24 * (isset($plugin_cookies->epg_cache_ttl) ? $plugin_cookies->epg_cache_ttl : 3);
                $cache_expired = filemtime($epg_cache_file) + $max_check_time;
                if ($cache_expired > time()) {
                    $program_epg = self::load_cache($epg_cache_file);
                    $from_cache = true;
                    hd_print(__METHOD__ . ": Loading all entries for EPG ID: '$epg_id' from file cache: $epg_cache_file");
                } else {
                    hd_print(__METHOD__ . ": Cache expired at $cache_expired now $now");
                }
            }

            if ($from_cache === false) {
                if ($epg_source === Plugin_Constants::EPG_INTERNAL) {
                    $xmltv_idx = isset($plugin_cookies->{Starnet_Epg_Setup_Screen::SETUP_ACTION_XMLTV_EPG_IDX})
                        ? $plugin_cookies->{Starnet_Epg_Setup_Screen::SETUP_ACTION_XMLTV_EPG_IDX}
                        : 'custom';

                    if (empty($this->xmltv_data)) {
                        $lock_file = get_temp_path(crc32($this->get_xmltv_url($xmltv_idx)) . ".lock");
                        if (file_exists($lock_file)) {
                            throw new Exception("Indexing in progress...");
                        }

                        throw new Exception("Empty or no EPG data for " . $channel->get_id());
                    }

                    hd_print(__METHOD__ . ": Fetching EPG ID: '$epg_id' from xmltv");
                    $program_epg = $this->get_epg_xmltv($epg_id, $xmltv_idx);
                } else {
                    hd_print(__METHOD__ . ": Fetching EPG ID: '$epg_id' from server");
                    $program_epg = self::get_epg_json($epg_url, $params);
                }
            }
            $counts = count($program_epg);
            if ($counts === 0) {
                throw new Exception("Empty or no EPG data for " . $channel->get_id());
            }

            if ($from_cache === false) {
                hd_print(__METHOD__ . ": Save EPG ID: '$epg_id' to file cache $epg_cache_file");
                self::save_cache($program_epg, $epg_cache_file);
            }

            hd_print(__METHOD__ . ": Total $counts EPG entries loaded");

            // filter out epg only for selected day
            $day_end_ts = $day_start_ts + 86400;

            //$date_start_l = format_datetime("Y-m-d H:i", $day_start_ts);
            //$date_end_l = format_datetime("Y-m-d H:i", $day_end_ts);
            //hd_print(__METHOD__ . ": Fetch entries for from: $date_start_l to: $date_end_l");

            $day_epg = array();
            foreach ($program_epg as $time_start => $entry) {
                if ($time_start >= $day_start_ts && $time_start < $day_end_ts) {
                    $day_epg[$time_start] = $entry;
                }
            }

            if (empty($day_epg)) {
                throw new Exception("No EPG data for " . $channel->get_id());
            }

            hd_print(__METHOD__ . ": Store day epg to memory cache");
            $this->epg_cache[$epg_id][$day_start_ts] = $day_epg;

            hd_print(__METHOD__ . ": Save day entries for EPG ID: '$epg_id' to file cache: $day_epg_cache");
            self::save_cache($day_epg, $day_epg_cache);

            return $day_epg;

        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Can't fetch EPG from $epg_source source: " . $ex->getMessage());
        }

        return false;
    }


    /**
     * @param $cached_file string
     * @return boolean|SplFileObject
     */
    protected static function open_xmltv_file($cached_file)
    {
        try {
            if (!file_exists($cached_file)) {
                throw new Exception("xmltv cache file not exist");
            }

            $file_object = new SplFileObject($cached_file);
            $file_object->setFlags(SplFileObject::DROP_NEW_LINE);
            $file_object->rewind();
        } catch(Exception $ex) {
            hd_print(__METHOD__ . ": " . $ex->getMessage());
            return false;
        }

        return $file_object;
    }

    /**
     * @param $id string
     * @param $idx
     * @return false|array
     */
    public function get_epg_xmltv($id, $idx)
    {
        if (!isset($this->xmltv_data[$id])) {
            hd_print(__METHOD__ . ": channel $id not found");
            return false;
        }

        $file_object = self::open_xmltv_file($this->get_xml_cached_file($idx));
        if (false === $file_object) {
            hd_print(__METHOD__ . ": Failed to open xmltv file by index: $idx");
            return false;
        }

        $ch_epg = array();
        foreach ($this->xmltv_data[$id] as $pos) {
            $xml_str = '';
            $file_object->fseek($pos);
            while (!$file_object->eof()) {
                $line = $file_object->fgets();
                $xml_str .= $line . PHP_EOL;
                if (strpos($line, "</programme") !== false) {
                    break;
                }
            }

            $xml_node = new DOMDocument();
            $xml_node->loadXML($xml_str);
            $xml = (array)simplexml_import_dom($xml_node);

            $program_start = strtotime((string)$xml['@attributes']['start']);
            $ch_epg[$program_start][Epg_Params::EPG_END] = strtotime((string)$xml['@attributes']['stop']);
            $ch_epg[$program_start][Epg_Params::EPG_NAME] = (string)$xml['title'];
            $ch_epg[$program_start][Epg_Params::EPG_DESC] = isset($xml['desc']) ? (string)$xml['desc'] : '';
        }

        ksort($ch_epg);
        return $ch_epg;
    }

    /**
     * @param $xmltv_url string
     * @param $xml_cached_path string
     * @param $max_check_time integer
     * @return array
     */
    public static function index_xmltv_file($xmltv_url, $xml_cached_path, $max_check_time)
    {
        $lock_file = get_temp_path(crc32($xmltv_url) . ".lock");
        if (file_exists($lock_file))
            return null;

        file_put_contents($lock_file, '');

        try {
            if (empty($xmltv_url)) {
                throw new Exception("xmltv url not defined");
            }

            $check_time_file = false;
            if (file_exists($xml_cached_path))
                $check_time_file = filemtime($xml_cached_path);

            $need_index = true;
            $xmltv_index_file = $xml_cached_path . '.idx';
            if (file_exists($xmltv_index_file)) {
                $data = json_decode(file_get_contents($xmltv_index_file), true);

                if (false !== $data) {
                    $need_index = false;
                    hd_print(__METHOD__ . ": load info from file '$xmltv_index_file'");
                    $xmltv_data = $data;
                }
            }

            if (empty($xmltv_data)) {
                hd_print(__METHOD__ . ": cached file not exist");
                $need_index = true;
                $xmltv_data = null;
            } else if (!$check_time_file || $check_time_file + $max_check_time < time()) {
                hd_print(__METHOD__ . ": cached file expired");
                $need_index = true;
                $xmltv_data = null;
            }

            if ($need_index || !file_exists($xml_cached_path)) {
                try {
                    $tmp_filename = get_temp_path(basename($xmltv_url));
                    $last_mod_file = HD::http_save_document($xmltv_url, $tmp_filename);
                    hd_print(__METHOD__ . ": Last changed time on server: " . date("Y-m-d H:s", $last_mod_file));
                    $need_index = true;

                    if (preg_match('|\.gz|', $xmltv_url)) {
                        hd_print(__METHOD__ . ": unpack $tmp_filename");
                        $gz = gzopen($tmp_filename, 'rb');
                        if (!$gz) {
                            throw new Exception("Failed to open $tmp_filename");
                        }

                        $dest = fopen($xml_cached_path, 'wb');
                        if (!$dest) {
                            throw new Exception("Failed to open $xml_cached_path");
                        }

                        $res = stream_copy_to_stream($gz, $dest);
                        gzclose($gz);
                        fclose($dest);
                        unlink($tmp_filename);
                        if ($res === false) {
                            throw new Exception("Failed to unpack $tmp_filename to $xml_cached_path");
                        }
                        hd_print(__METHOD__ . ": $res bytes written to $xml_cached_path");
                    } else {
                        rename($tmp_filename, $xml_cached_path);
                    }

                } catch (Exception $ex) {
                    throw new Exception("Can't download file: $xmltv_url");
                }
            }

            if ($need_index) {
                if (!file_exists($xml_cached_path)) {
                    throw new Exception("xmltv cache file not exist");
                }

                hd_print(__METHOD__ . ": Reindexing...");
                $t = microtime(1);

                $file_object = new SplFileObject($xml_cached_path);
                $file_object->setFlags(SplFileObject::DROP_NEW_LINE);
                $file_object->rewind();

                while (!$file_object->eof()) {
                    $pos = $file_object->ftell();
                    $line = $file_object->fgets();
                    if (strpos($line, "<programme") === false)
                        continue;

                    $ch_start = strpos($line, 'channel="');
                    if ($ch_start === false)
                        continue;

                    $ch_end = strpos($line, '"', $ch_start + 9);
                    if ($ch_end === false) {
                        continue;
                    }

                    $ch_start += 9;
                    $channel = substr($line, $ch_start, $ch_end - $ch_start);
                    $xmltv_data[$channel][] = $pos;
                }

                hd_print(__METHOD__ . ": Indexing XMLTV done: " . (microtime(1) - $t) . " secs");
                file_put_contents($xmltv_index_file, json_encode($xmltv_data));
            }
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": " . $ex->getMessage());
            unlink($lock_file);
            return null;
        }

        unlink($lock_file);

        return $xmltv_data;
    }

    /**
     * clear memory cache
     */
    public function clear_epg_cache($all = false)
    {
        $this->epg_cache = array();

        $cache_dir = get_temp_path(self::EPG_CACHE_PATH);
        hd_print(__METHOD__ . ": clear epg file cache: $cache_dir");
        foreach (glob($cache_dir . "*") as $file) {
            if (is_file($file)) {
                unlink($file);
            }
        }

        if ($all) {
            $this->xmltv_data = null;
            $xcache_dir = get_temp_path(self::EPG_XMLTV_CACHE_PATH);
            hd_print(__METHOD__ . ": clear xmltv cache: $xcache_dir");
            foreach (glob($xcache_dir . "*") as $file) {
                if (is_file($file)) {
                    unlink($file);
                }
            }
        }
    }

    /**
     * @param array $day_epg
     * @param string $cache_file
     */
    protected static function save_cache($day_epg, $cache_file)
    {
        if (!empty($day_epg)) {
            ksort($day_epg, SORT_NUMERIC);
            file_put_contents($cache_file, serialize($day_epg));
        }
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
        $channel_epg = array();

        try {
            $ch_data = HD::DownloadJson($url);
            if (empty($ch_data)) {
                hd_print(__METHOD__ . ": Empty document returned.");
                return $channel_epg;
            }
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": http exception: " . $ex->getMessage());
            return $channel_epg;
        }

        if (!empty($parser_params[Epg_Params::EPG_ROOT])) {
            foreach (explode('|', $parser_params[Epg_Params::EPG_ROOT]) as $level) {
                $epg_root = trim($level, "[]");
                $ch_data = $ch_data[$epg_root];
            }
        }
        //hd_print("json epg root: " . $parser_params[EPG_ROOT]);
        //hd_print("json start: " . $parser_params[EPG_START]);
        //hd_print("json end: " . $parser_params[EPG_END]);
        //hd_print("json title: " . $parser_params[EPG_NAME]);
        //hd_print("json desc: " . $parser_params[EPG_DESC]);

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
                    array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY, Plugin_Macros::HOUR, Plugin_Macros::MIN),
                    array('Y', 'm', 'd', 'H', 'i'),
                    $parser_params[Epg_Params::EPG_TIME_FORMAT]);
                $dt = DateTime::createFromFormat($time_format, $program_start, new DateTimeZone('UTC'));
                $program_start = $dt->getTimestamp() - $parser_params[Epg_Params::EPG_TIMEZONE] * 3600; // subtract real EPG timezone
            }

            // prefill data to avoid undefined index notice
            $channel_epg[$program_start][Epg_Params::EPG_END] = 0;
            $channel_epg[$program_start][Epg_Params::EPG_NAME] = '';
            $channel_epg[$program_start][Epg_Params::EPG_DESC] = '';

            if ($use_duration) {
                $channel_epg[$program_start][Epg_Params::EPG_END] = $program_start + (int)$entry[$parser_params[Epg_Params::EPG_END]];
            } else if ($no_end) {
                if ($prev_start !== 0) {
                    $channel_epg[$prev_start][Epg_Params::EPG_END] = $program_start;
                }
                $prev_start = $program_start;
            } else {
                $channel_epg[$program_start][Epg_Params::EPG_END] = (int)$entry[$parser_params[Epg_Params::EPG_END]];
            }

            if (isset($entry[$parser_params[Epg_Params::EPG_NAME]])) {
                $channel_epg[$program_start][Epg_Params::EPG_NAME] = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_NAME]]);
            }
            if (isset($entry[$parser_params[Epg_Params::EPG_DESC]])) {
                $desc = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_DESC]]);
                $desc = str_replace('<br>', PHP_EOL, $desc);
                $channel_epg[$program_start][Epg_Params::EPG_DESC] = $desc;
            }
        }

        if ($no_end && $prev_start !== 0) {
            $channel_epg[$prev_start][Epg_Params::EPG_END] = $prev_start + 3600; // fake end
        }

        ksort($channel_epg);
        return $channel_epg;
    }
}

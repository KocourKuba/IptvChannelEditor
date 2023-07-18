<?php
require_once 'hd.php';

class Epg_Manager
{
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
    }

    public function clear_xmltv_urls()
    {
        $this->xmltv_urls = array();
        $this->xml_cached_file = array();
    }

    public function clear_epg_memory_cache()
    {
        $this->epg_cache = array();
        $this->xmltv_data = null;
    }

    /**
     * @param $idx string
     * @param $xmltv_url string
     */
    public function set_xmltv_url($idx, $xmltv_url)
    {
        $this->xmltv_urls[$idx] = $xmltv_url;
        $this->xml_cached_file[$idx] = $this->config->plugin_info['app_name'] . "_" . hash('crc32', $xmltv_url) . ".xmltv";
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
     * @return string
     */
    public function get_xmltv_idx($plugin_cookies)
    {

        if (isset($plugin_cookies->{Starnet_Epg_Setup_Screen::SETUP_ACTION_XMLTV_EPG_IDX}))
            return $plugin_cookies->{Starnet_Epg_Setup_Screen::SETUP_ACTION_XMLTV_EPG_IDX};

        return array_search(reset($this->xmltv_urls), $this->xmltv_urls);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public static function get_xcache_dir($plugin_cookies)
    {
        $xcache_dir = get_paved_path(smb_tree::get_folder_info($plugin_cookies, PARAM_XMLTV_CACHE_PATH, get_data_path("epg_cache")));
        if (substr($xcache_dir, -1) !== '/') {
            $xcache_dir .= '/';
        }
        return $xcache_dir;
    }

    /**
     * @param $plugin_cookies
     * @param $data MediaURL
     */
    public static function set_xcache_dir($plugin_cookies, $data)
    {
        smb_tree::set_folder_info($plugin_cookies, $data, PARAM_XMLTV_CACHE_PATH);
    }

    /**
     * @param $idx string
     * @param $plugin_cookies
     * @return string
     */
    public function get_xml_cached_file($idx, $plugin_cookies)
    {
        return isset($this->xml_cached_file[$idx]) ? self::get_xcache_dir($plugin_cookies) . $this->xml_cached_file[$idx] : '';
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
            if ($epg_source === Plugin_Constants::EPG_INTERNAL) {
                $params[Epg_Params::EPG_DOMAIN] = $this->get_xmltv_url($this->get_xmltv_idx($plugin_cookies));
            } else if (empty($params[Epg_Params::EPG_URL])) {
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
            hd_print(__METHOD__ . ": Epg url: $epg_url");
            $hash = hash('crc32', $epg_url);
            $epg_cache_file = self::get_xcache_dir($plugin_cookies) . "$hash.cache";
            $program_epg = array();
            if ($epg_source === Plugin_Constants::EPG_INTERNAL) {
                hd_print(__METHOD__ . ": Fetching EPG ID: '$epg_id' from xmltv");
                $program_epg = $this->get_epg_xmltv($epg_id, $this->get_xmltv_idx($plugin_cookies), $plugin_cookies);
            } else {
                $from_cache = false;
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
                        unlink($epg_cache_file);
                    }
                }

                if ($from_cache === false) {
                    hd_print(__METHOD__ . ": Fetching EPG ID: '$epg_id' from server");
                    $program_epg = self::get_epg_json($epg_url, $params);
                    hd_print(__METHOD__ . ": Save EPG ID: '$epg_id' to file cache $epg_cache_file");
                    self::save_cache($program_epg, $epg_cache_file);
                }
            }

            $counts = count($program_epg);
            if ($counts === 0) {
                throw new Exception("Empty or no EPG data for " . $channel->get_id());
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

            return $day_epg;

        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Can't fetch EPG from $epg_source source: " . $ex->getMessage());
        }

        return false;
    }


    /**
     * @param $cached_file string
     * @return SplFileObject
     * @throws Exception
     */
    protected static function open_xmltv_file($cached_file)
    {
        if (!file_exists($cached_file)) {
            throw new Exception("xmltv cache file not exist");
        }

        $file_object = new SplFileObject($cached_file);
        $file_object->setFlags(SplFileObject::DROP_NEW_LINE);
        $file_object->rewind();

        return $file_object;
    }

    /**
     * @param $id string
     * @param $idx
     * @param $plugin_cookies
     * @return false|array
     */
    public function get_epg_xmltv($id, $idx, $plugin_cookies)
    {
        try {

            $cached_file = $this->get_xml_cached_file($idx, $plugin_cookies);
            if (empty($cached_file)) {
                throw new Exception("xmltv file for index: $idx is not set");
            }

            if (!isset($this->xmltv_data[$id])) {
                $channel_idx = self::get_xcache_dir($plugin_cookies) . $id . "_" . basename($cached_file) . ".index";
                if (!file_exists($channel_idx)) {
                    throw new Exception("channel $id not found");
                }

                $this->xmltv_data[$id] = json_decode(file_get_contents($channel_idx), true);
            }


            $file_object = self::open_xmltv_file($cached_file);
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
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": " . $ex->getMessage());
            return false;
        }

        return $ch_epg;
    }

    /**
     * @param $xml_cached_path
     * @param $max_check_time
     * @return boolean
     */
    public static function is_xmltv_cache_valid($xml_cached_path, $max_check_time)
    {
        hd_print(__METHOD__ . ": Cached file path: $xml_cached_path");
        if (!file_exists($xml_cached_path)) {
            hd_print(__METHOD__ . ": Cached file path not exist");
            return false;
        }

        $check_time_file = filemtime($xml_cached_path);
        if ($check_time_file && $check_time_file + $max_check_time > time()) {
            hd_print(__METHOD__ . ": Cached file index: $xml_cached_path not expired " . date("Y-m-d H:s", $check_time_file));
            return true;
        }
        unlink($xml_cached_path);

        return false;
    }

    /**
     * @param $xmltv_url string
     * @param $xml_cached_path string
     * @return boolean|string
     */
    public static function download_xmltv_url($xmltv_url, $xml_cached_path)
    {
        if (file_exists($xml_cached_path)) {
            return true;
        }

        hd_print(__METHOD__ . ": Storage space in cache dir: " . HD::get_storage_size(dirname($xml_cached_path)));

        try {
            if (empty($xmltv_url)) {
                throw new Exception("XMTLV EPG url not set");
            }

            $tmp_filename = get_temp_path(basename($xmltv_url));
            $last_mod_file = HD::http_save_document($xmltv_url, $tmp_filename);
            hd_print(__METHOD__ . ": Last changed time on server: " . date("Y-m-d H:s", $last_mod_file));

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
            hd_print(__METHOD__ . ": " . $ex->getMessage());
            return $ex->getMessage();
        }

        return true;
    }

    /**
     * @param $xml_cached_path string
     * @param $epg_ids array
     */
    public static function index_xmltv_file($xml_cached_path, $epg_ids)
    {
        try {
            $lock_file = $xml_cached_path . ".lock";
            if (file_exists($lock_file)) {
                throw new Exception("File indexed now, skipped");
            }

            file_put_contents($lock_file, '');
            hd_print(__METHOD__ . ": Reindexing $xml_cached_path");
            $t = microtime(1);

            $count = 0;
            $parent_name = basename($xml_cached_path);
            $parent_path = dirname($xml_cached_path);
            $file_object = self::open_xmltv_file($xml_cached_path);
            $prev_channel = '';
            $xmltv_data = array();
            while (!$file_object->eof()) {
                $pos = $file_object->ftell();
                $line = $file_object->fgets();
                if (strpos($line, "<programme") === false) {
                    continue;
                }

                $ch_start = strpos($line, 'channel="');
                if ($ch_start === false) {
                    continue;
                }

                $ch_end = strpos($line, '"', $ch_start + 9);
                if ($ch_end === false) {
                    continue;
                }

                $ch_start += 9;
                $channel = substr($line, $ch_start, $ch_end - $ch_start);

                // if epg id not present in current channels list skip it
                if (!isset($epg_ids[$channel])) {
                    continue;
                }

                if ($prev_channel !== $channel) {
                    if (empty($prev_channel)) {
                        $prev_channel = $channel;
                    } else {
                        if (!empty($xmltv_data)) {
                            file_put_contents($parent_path . "/$prev_channel" . "_$parent_name.index", json_encode($xmltv_data));
                            $count++;
                        }

                        $prev_channel = $channel;
                        $xmltv_data = array();
                    }
                }
                $xmltv_data[] = $pos;
            }

            if (!empty($prev_channel) && !empty($xmltv_data)) {
                file_put_contents($parent_path . "/$prev_channel" . "_$parent_name.index", json_encode($xmltv_data));
            }

            hd_print(__METHOD__ . ": Reindexing XMLTV done: " . (microtime(1) - $t) . " secs");
            hd_print(__METHOD__ . ": Total epg id's indexed: $count");
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": " . $ex->getMessage());
        }

        HD::ShowMemoryUsage();
        hd_print(__METHOD__ . ": Storage space in cache dir after reindexing: " . HD::get_storage_size(dirname($xml_cached_path)));

        unlink($lock_file);
    }

    /**
     * clear memory cache
     * @param $plugin_cookies
     */
    public function clear_epg_cache($plugin_cookies)
    {
        $this->epg_cache = array();
        $this->xmltv_data = null;

        $dir = self::get_xcache_dir($plugin_cookies);
        hd_print(__METHOD__ . ": clear cache dir: $dir");
        $files = array_diff(scandir($dir), array('.','..'));
        foreach ($files as $file) {
            if (!is_dir("$dir/$file")) {
                unlink("$dir/$file");
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

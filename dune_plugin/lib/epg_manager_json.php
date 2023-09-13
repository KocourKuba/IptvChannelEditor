<?php
/**
 * The MIT License (MIT)
 *
 * @Author: sharky72 (https://github.com/KocourKuba)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

require_once 'hd.php';
require_once 'epg_params.php';

class Epg_Manager_Json extends Epg_Manager
{
    /**
     * @var string
     */
    protected $index_ext = '.cache';

    /**
     * @var Default_Dune_Plugin
     */
    protected $plugin;

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
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct($plugin)
    {
        parent::__construct($plugin->config->plugin_info['app_version'], $plugin->get_cache_dir(), '');
        $this->plugin = $plugin;
    }

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param Channel $channel
     * @param int $day_start_ts
     * @return array|false
     */
    public function get_day_epg_items(Channel $channel, $day_start_ts)
    {
        $epg_ids = $channel->get_epg_ids();
        if (empty($epg_ids)) {
            hd_debug_print("EPG ID not defined");
            return false;
        }

        $day_epg = array();
        $first = reset($epg_ids);
        foreach (array(Plugin_Constants::EPG_FIRST, Plugin_Constants::EPG_SECOND) as $key => $epg_source) {
            $params = $this->plugin->config->get_epg_params($epg_source);
            if (empty($params[Epg_Params::EPG_URL])) continue;

            $epg_id = isset($epg_ids[$key]) ? $epg_ids[$key] : $first;

            if (isset($this->epg_cache[$epg_id][$day_start_ts])) {
                hd_debug_print("Load day EPG ID $epg_id ($day_start_ts) from memory cache ");
                return $this->epg_cache[$epg_id][$day_start_ts];
            }

            $channel_id = $channel->get_id();
            $channel_title = $channel->get_title();
            hd_debug_print("Try to load EPG ID: '$epg_id' for channel '$channel_id' ($channel_title)");

            $this->epg_url = $params[Epg_Params::EPG_URL];
            $this->url_hash = (empty($this->epg_url) ? '' : Hashed_Array::hash($this->epg_url));

            $epg_url = str_replace(
                array(Plugin_Macros::API_URL,
                    Plugin_Macros::EPG_DOMAIN,
                    Plugin_Macros::EPG_ID,
                    Plugin_Macros::ID,
                    Plugin_Macros::DUNE_IP
                ),
                array($this->plugin->config->get_feature(Plugin_Constants::PROVIDER_API_URL),
                    $params[Epg_Params::EPG_DOMAIN],
                    str_replace(' ', '%20', $epg_id),
                    $channel_id,
                    $this->dune_ip
                ),
                $this->epg_url);

            if (strpos($epg_url, Plugin_Macros::DATE) !== false) {
                $date_format = str_replace(
                    array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY),
                    array('Y', 'm', 'd'),
                    $params[Epg_Params::EPG_DATE_FORMAT]);

                $epg_date = gmdate($date_format, $day_start_ts + get_local_time_zone_offset());
                $epg_url = str_replace(Plugin_Macros::DATE, $epg_date, $epg_url);
                //hd_debug_print("From DATE: $epg_date");
            }

            if (strpos($epg_url, Plugin_Macros::TIMESTAMP) !== false) {
                $epg_url = str_replace(Plugin_Macros::TIMESTAMP, $day_start_ts, $epg_url);
                //hd_debug_print("From Timestamp: $day_start_ts");
            }

            if (strpos($epg_url, Plugin_Macros::TOKEN) !== false) {
                $epg_url = str_replace(Plugin_Macros::TOKEN, $this->plugin->get_credentials(Ext_Params::M_TOKEN), $epg_url);
            }

            $epg_url = str_replace('#', '%23', $epg_url);
            $hash = hash('crc32', $epg_url);

            $epg_cache_file = $this->get_cache_stem("_$hash$this->index_ext");
            $from_cache = false;
            $all_epg = array();
            if (file_exists($epg_cache_file)) {
                $now = time();
                $max_check_time = 3600 * 24 * $this->plugin->get_parameter(PARAM_EPG_CACHE_TTL, 3);
                $cache_expired = filemtime($epg_cache_file) + $max_check_time;
                if ($cache_expired > time()) {
                    $all_epg = unserialize(file_get_contents($epg_cache_file));
                    $from_cache = true;
                    hd_debug_print("Loading all entries for EPG ID: '$epg_id' from file cache: $epg_cache_file");
                } else {
                    hd_debug_print("Cache expired at $cache_expired now $now");
                    unlink($epg_cache_file);
                }
            }

            if ($from_cache === false) {
                hd_debug_print("Fetching EPG ID: '$epg_id' from server");
                $all_epg = self::get_epg_json($epg_url, $params);
                if (!empty($all_epg)) {
                    hd_debug_print("Save EPG ID: '$epg_id' to file cache $epg_cache_file");
                    file_put_contents($epg_cache_file, serialize($all_epg));
                }
            }

            $counts = count($all_epg);
            if ($counts === 0) {
                hd_debug_print("Empty or no EPG data for " . $channel->get_id());
                continue;
            }

            hd_debug_print("Total $counts EPG entries loaded");

            // filter out epg only for selected day
            $day_end_ts = $day_start_ts + 86400;

            if (LogSeverity::$is_debug) {
                $date_start_l = format_datetime("Y-m-d H:i", $day_start_ts);
                $date_end_l = format_datetime("Y-m-d H:i", $day_end_ts);
                hd_debug_print("Fetch entries for from: $date_start_l to: $date_end_l");
            }

            foreach ($all_epg as $time_start => $entry) {
                if ($time_start >= $day_start_ts && $time_start < $day_end_ts) {
                    $day_epg[$time_start] = $entry;
                }
            }

            if (empty($day_epg)) {
                hd_debug_print("No EPG data for " . $channel->get_id());
                continue;
            }

            hd_debug_print("Store day epg to memory cache");
            $this->epg_cache[$epg_id][$day_start_ts] = $day_epg;

            return $day_epg;
        }

        if (($this->flags & EPG_FAKE_EPG) && $channel->get_archive() !== 0) {
            hd_debug_print("Create fake data for non existing EPG data");
            for ($start = $day_start_ts, $n = 1; $start <= $day_start_ts + 86400; $start += 3600, $n++) {
                $day_epg[$start][Epg_Params::EPG_END] = $start + 3600;
                $day_epg[$start][Epg_Params::EPG_NAME] = TR::load_string('fake_epg_program') . " $n";
                $day_epg[$start][Epg_Params::EPG_DESC] = '';
            }
        } else {
            hd_debug_print("No EPG for channel");
        }

        return $day_epg;
    }

    /**
     * request server for epg and parse json response
     * @param string $url
     * @param array $parser_params
     * @return array
     */
    protected static function get_epg_json($url, $parser_params)
    {
        $channel_epg = array();

        try {
            $ch_data = HD::DownloadJson($url);
            if (empty($ch_data)) {
                hd_debug_print("Empty document returned.");
                return $channel_epg;
            }
        } catch (Exception $ex) {
            hd_debug_print("http exception: " . $ex->getMessage());
            return $channel_epg;
        }

        if (!empty($parser_params[Epg_Params::EPG_ROOT])) {
            foreach (explode('|', $parser_params[Epg_Params::EPG_ROOT]) as $level) {
                $epg_root = trim($level, "[]");
                $ch_data = $ch_data[$epg_root];
            }
        }

        hd_debug_print("json epg root: " . $parser_params[Epg_Params::EPG_ROOT], true);
        hd_debug_print("json start: " . $parser_params[Epg_Params::EPG_START], true);
        hd_debug_print("json end: " . $parser_params[Epg_Params::EPG_END], true);
        hd_debug_print("json title: " . $parser_params[Epg_Params::EPG_NAME], true);
        hd_debug_print("json desc: " . $parser_params[Epg_Params::EPG_DESC], true);

        // collect all program that starts after day start and before day end
        $prev_start = 0;
        $no_end = empty($parser_params[Epg_Params::EPG_END]);
        $no_timestamp = !empty($parser_params[Epg_Params::EPG_TIME_FORMAT]);
        $use_duration = $parser_params[Epg_Params::EPG_USE_DURATION];
        foreach ($ch_data as $entry) {
            $program_start = $entry[$parser_params[Epg_Params::EPG_START]];
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

        ksort($channel_epg, SORT_NUMERIC);
        return $channel_epg;
    }
}

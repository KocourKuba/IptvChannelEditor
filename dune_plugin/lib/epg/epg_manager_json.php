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

require_once 'epg_manager_xmltv.php';

class Epg_Manager_Json extends Epg_Manager_Xmltv
{
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
     * @inheritDoc
     * @override
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
            $epg_url = $this->plugin->config->get_epg_parameter($epg_source, Epg_Params::EPG_URL);
            if (empty($epg_url)) continue;

            $epg_id = isset($epg_ids[$key]) ? $epg_ids[$key] : $first;

            if (isset($this->epg_cache[$epg_id][$day_start_ts])) {
                hd_debug_print("Load day EPG ID $epg_id ($day_start_ts) from memory cache ");
                return $this->epg_cache[$epg_id][$day_start_ts];
            }

            $channel_id = $channel->get_id();
            $channel_title = $channel->get_title();
            hd_debug_print("Try to load EPG ID: '$epg_id' for channel '$channel_id' ($channel_title)");

            $epg_url = $this->plugin->config->replace_account_vars($epg_url);

            $epg_id = str_replace(array('%28', '%29'), array('(', ')'), rawurlencode($epg_id));

            $epg_url = str_replace(
                array(Plugin_Macros::EPG_DOMAIN,
                    Plugin_Macros::EPG_ID,
                    Plugin_Macros::ID,
                    Plugin_Macros::DUNE_IP
                ),
                array($this->plugin->config->get_epg_parameter($epg_source, Epg_Params::EPG_DOMAIN),
                    $epg_id,
                    $channel_id,
                    $this->dune_ip
                ),
                $epg_url);

            if (strpos($epg_url, Plugin_Macros::DATE) !== false) {
                $date_format = str_replace(
                    array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY),
                    array('Y', 'm', 'd'),
                    $this->plugin->config->get_epg_parameter($epg_source, Epg_Params::EPG_DATE_FORMAT));

                $epg_date = gmdate($date_format, $day_start_ts + get_local_time_zone_offset());
                $epg_url = str_replace(Plugin_Macros::DATE, $epg_date, $epg_url);
            }

            if (strpos($epg_url, Plugin_Macros::TIMESTAMP) !== false) {
                $epg_url = str_replace(Plugin_Macros::TIMESTAMP, $day_start_ts, $epg_url);
            }

            $epg_url = str_replace('#', '%23', $epg_url);
            $epg_cache_file = get_temp_path(Hashed_Array::hash($epg_url) . ".cache");
            $from_cache = false;
            $all_epg = array();
            if (file_exists($epg_cache_file)) {
                $now = time();
                $mtime = filemtime($epg_cache_file);
                $cache_expired = $mtime + $this->plugin->get_setting(PARAM_EPG_CACHE_TIME, 1) * 3600;
                if ($cache_expired > time()) {
                    $all_epg = parse_json_file($epg_cache_file);
                    $from_cache = true;
                    hd_debug_print("Loading all entries for EPG ID: '$epg_id' from file cache: $epg_cache_file");
                } else {
                    hd_debug_print("EPG cache $epg_cache_file expired " . ($now - $cache_expired) . " sec ago. Timestamp $mtime. Remove cache file");
                    unlink($epg_cache_file);
                }
            }

            if ($from_cache === false) {
                hd_debug_print("Fetching EPG ID: '$epg_id' from server: $epg_url");
                $all_epg = self::get_epg_json($epg_url, $this->plugin->config->get_epg_parameters($epg_source));
                if (!empty($all_epg)) {
                    hd_debug_print("Save EPG ID: '$epg_id' to file cache $epg_cache_file");
                    store_to_json_file($epg_cache_file, $all_epg);
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
                $day_epg[$start][Epg_Params::EPG_NAME] = TR::load('fake_epg_program') . " $n";
                $day_epg[$start][Epg_Params::EPG_DESC] = '';
            }
        } else {
            hd_debug_print("No EPG for channel");
        }

        return $day_epg;
     }

    public function clear_epg_cache()
    {
        $this->epg_cache = array();
        $files = get_temp_path('*.cache');
        hd_debug_print("clear cache files: $files");
        shell_exec('rm -f ' . $files);
        clearstatcache();
    }
    ///////////////////////////////////////////////////////////////////////////////
    /// protected methods

    /**
     * request server for epg and parse json response
     * @param string $url
     * @param array $parser_params
     * @return array
     */
    protected static function get_epg_json($url, $parser_params)
    {
        $channel_epg = array();

        if (empty($parser_params)) {
            return $channel_epg;
        }

        hd_debug_print("parser params: " . json_encode($parser_params), true);

        try {
            $doc = HD::http_get_document($url);
            $ch_data = json_decode($doc, true);
            if (empty($ch_data)) {
                hd_debug_print("failed to decode json: $doc");
                return $channel_epg;
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return $channel_epg;
        }

        if (!empty($parser_params[Epg_Params::EPG_ROOT])) {
            foreach (explode('|', $parser_params[Epg_Params::EPG_ROOT]) as $level) {
                $epg_root = trim($level, "[]");
                $ch_data = $ch_data[$epg_root];
            }
        }

        // Possible need to add this to setup
        // disabling end can help problem with overlapping end/start EPG
        $parser_params[Epg_Params::EPG_END] = '';

        hd_debug_print("json epg root: " . $parser_params[Epg_Params::EPG_ROOT], true);
        hd_debug_print("json start: " . $parser_params[Epg_Params::EPG_START], true);
        hd_debug_print("json title: " . $parser_params[Epg_Params::EPG_NAME], true);
        hd_debug_print("json desc: " . $parser_params[Epg_Params::EPG_DESC], true);
        if (isset($parser_params[Epg_Params::EPG_ICON])) {
            hd_debug_print("json icon: " . $parser_params[Epg_Params::EPG_ICON], true);
        }

        // collect all program that starts after day start and before day end
        $prev_start = 0;
        foreach ($ch_data as $entry) {
            if (!isset($entry[$parser_params[Epg_Params::EPG_START]])) continue;

            $program_start = $entry[$parser_params[Epg_Params::EPG_START]];

            if ($prev_start !== 0) {
                $channel_epg[$prev_start][Epg_Params::EPG_END] = $program_start;
            }
            $prev_start = $program_start;

            if (isset($entry[$parser_params[Epg_Params::EPG_NAME]])) {
                $channel_epg[$program_start][Epg_Params::EPG_NAME] = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_NAME]]);
            } else {
                $channel_epg[$program_start][Epg_Params::EPG_NAME] = '';
            }

            if (isset($entry[$parser_params[Epg_Params::EPG_DESC]])) {
                $desc = HD::unescape_entity_string($entry[$parser_params[Epg_Params::EPG_DESC]]);
                $desc = str_replace('<br>', PHP_EOL, $desc);
                $channel_epg[$program_start][Epg_Params::EPG_DESC] = $desc;
            } else {
                $channel_epg[$program_start][Epg_Params::EPG_DESC] = '';
            }

            if (isset($parser_params[Epg_Params::EPG_ICON], $entry[$parser_params[Epg_Params::EPG_ICON]])) {
                $channel_epg[$program_start][Epg_Params::EPG_ICON] = $entry[$parser_params[Epg_Params::EPG_ICON]];
            } else {
                $channel_epg[$program_start][Epg_Params::EPG_ICON] = '';
            }
        }

        if ($prev_start !== 0) {
            $channel_epg[$prev_start][Epg_Params::EPG_END] = $prev_start + 3600; // fake end
        }

        ksort($channel_epg, SORT_NUMERIC);
        return $channel_epg;
    }
}

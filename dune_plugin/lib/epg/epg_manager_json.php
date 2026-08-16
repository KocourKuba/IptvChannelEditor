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
    const /* (int)	  */ EPG_PARAM = 'epg_param';
    const /* (char *) */ EPG_DOMAIN = 'epg_domain';
    const /* (char *) */ EPG_URL = 'epg_url';
    const /* (char *) */ EPG_ROOT = 'epg_root';
    const /* (int)	  */ EPG_START = 'epg_start';
    const /* (int)	  */ EPG_END = 'epg_end';
    const /* (char *) */ EPG_NAME = 'epg_name';
    const /* (char *) */ EPG_DESC = 'epg_desc';
    const /* (char *) */ EPG_ICON = 'epg_icon';
    const /* (char *) */ EPG_DATE_FORMAT = 'epg_date_format';
    const /* (char *) */ EPG_TIME_FORMAT = 'epg_time_format';
    const /* (char *) */ EPG_USE_DURATION = 'epg_use_duration';
    const /* (char *) */ EPG_TIMEZONE = 'epg_timezone';

    /**
     * contains current dune IP
     * @var string
     */
    protected $dune_ip;

    /**
     * contains known channels and aliases
     * @var array
     */
    protected $ch_info_cache = array();

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

        if (!$this->plugin->is_json_capable()) {
            return array($day_start_ts => array(
                PluginTvEpgProgram::end_tm_sec => $day_start_ts + 86400,
                PluginTvEpgProgram::name => TR::load('epg_not_exist'),
                PluginTvEpgProgram::description => TR::load('epg_not_capable'),
            ));
        }

        $day_epg = array();
        $first = reset($epg_ids);
        foreach (array(Plugin_Constants::EPG_FIRST, Plugin_Constants::EPG_SECOND) as $key => $epg_source) {
            $channel_id = $channel->get_id();
            $channel_title = $channel->get_title();
            $epg_id = isset($epg_ids[$key]) ? $epg_ids[$key] : $first;

            hd_debug_print("Try to load EPG ID: '$epg_id' for channel '$channel_id' ($channel_title)");

            if (isset($this->epg_cache[$epg_id][$day_start_ts])) {
                hd_debug_print("Load day EPG ID $epg_id ($day_start_ts) from memory cache ");
                return $this->epg_cache[$epg_id][$day_start_ts];
            }

            $epg_template_url = $this->plugin->config->get_epg_parameter($epg_source, self::EPG_URL);
            $template_hash = Hashed_Array::hash($epg_template_url);
            $host = parse_url($epg_template_url, PHP_URL_HOST);
            if ($host === "epg.esalecrm.com" || $host === "epg.esalecrm.net") {
                if (empty($this->ch_info_cache[$template_hash])) {
                    $this->ch_info_cache[$template_hash] = self::get_channels_info($epg_template_url);
                }

                $all_info = $this->ch_info_cache[$template_hash];
                if (empty($all_info)) {
                    hd_debug_print("Known list empty! No matching performed.", true);
                } else if (in_array($epg_id, $all_info['epg_id'])) {
                    hd_debug_print("EPG ID: $epg_id is known. Continue fetching.", true);
                } else {
                    // this epg id is not known, try to find it in aliases (lower case)
                    // channel info array contains alias as key and mapped epg id as value
                    hd_debug_print("EPG ID '$epg_id' not found in known list", true);
                    $alias = mb_convert_case($channel_title, MB_CASE_LOWER, "UTF-8");
                    if (!array_key_exists($alias, $all_info['epg_aliases'])) {
                        // EPG not found by EPG ID and channel name
                        hd_debug_print("No EPG id found in known aliases list", true);
                        continue;
                    }

                    $epg_id_subst = $all_info['epg_aliases'][$alias];
                    hd_debug_print("EPG ID for '$alias' found in known list: $epg_id_subst", true);
                    $epg_id = $epg_id_subst;
                }
            }

            $epg_url = $this->get_egp_url($epg_source, $epg_id, $channel_id, $day_start_ts);
            if (empty($epg_url)) continue;

            $epg_cache_file = get_data_path('/epg_cache/' . Hashed_Array::hash($epg_url) . ".cache");
            $from_cache = false;
            $all_epg = array();
            if (file_exists($epg_cache_file)) {
                $now = time();
                $mtime = filemtime($epg_cache_file);
                $cache_expired = $mtime + $this->plugin->get_setting(PARAM_EPG_CACHE_TIME, 4) * 3600;
                if ($cache_expired > time()) {
                    $all_epg = parse_json_file($epg_cache_file);
                    $from_cache = true;
                    hd_debug_print("Loading all entries for EPG ID: '$epg_id' from file cache: $epg_cache_file");
                } else {
                    hd_debug_print("EPG cache $epg_cache_file expired " . ($now - $cache_expired) . " sec ago. Timestamp $mtime. Remove cache file");
                    safe_unlink($epg_cache_file);
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

            break;
        }

        if (empty($day_epg)) {
            hd_debug_print("No EPG for channel");
            $day_epg = $this->getFakeEpg($channel, $day_start_ts);
        }

        return $day_epg;
    }

     public function get_egp_url($epg_source, $epg_id, $channel_id, $day_start_ts)
     {
         $epg_url = $this->plugin->config->get_epg_parameter($epg_source, self::EPG_URL);
         if (empty($epg_url)) {
             return '';
         }

         $epg_id = str_replace(array('%28', '%29'), array('(', ')'), rawurlencode($epg_id));
         $epg_url = $this->plugin->config->replace_account_vars($epg_url);
         $epg_url = str_replace(
             array(Plugin_Macros::EPG_DOMAIN,
                 Plugin_Macros::EPG_ID,
                 Plugin_Macros::ID,
                 Plugin_Macros::DUNE_IP
             ),
             array($this->plugin->config->get_epg_parameter($epg_source, self::EPG_DOMAIN),
                 $epg_id,
                 $channel_id,
                 $this->dune_ip
             ),
             $epg_url);

         if (strpos($epg_url, Plugin_Macros::DATE) !== false) {
             $date_format = str_replace(
                 array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY),
                 array('Y', 'm', 'd'),
                 $this->plugin->config->get_epg_parameter($epg_source, self::EPG_DATE_FORMAT));

             $epg_date = gmdate($date_format, $day_start_ts + get_local_time_zone_offset());
             $epg_url = str_replace(Plugin_Macros::DATE, $epg_date, $epg_url);
         }

         if (strpos($epg_url, Plugin_Macros::TIMESTAMP) !== false) {
             $epg_url = str_replace(Plugin_Macros::TIMESTAMP, $day_start_ts, $epg_url);
         }

         $epg_url = str_replace('#', '%23', $epg_url);
         hd_debug_print("EPG URL: $epg_url");
         return $epg_url;
     }

    public function clear_epg_cache()
    {
        $this->ch_info_cache = array();
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

        $param_epg_root = safe_get_value($parser_params, self::EPG_ROOT);
        $param_epg_start = safe_get_value($parser_params, self::EPG_START);
        $param_epg_name = safe_get_value($parser_params, self::EPG_NAME);
        $param_epg_desc = safe_get_value($parser_params, self::EPG_DESC);
        $param_epg_icon = safe_get_value($parser_params, self::EPG_ICON);
        $param_epg_time_format = safe_get_value($parser_params, self::EPG_TIME_FORMAT);
        $param_epg_timezone = safe_get_value($parser_params, self::EPG_TIMEZONE);

        try {
            $doc = Curl_Wrapper::getInstance()->download_content($url);
            $ch_data = json_decode($doc, true);
            if (empty($ch_data)) {
                hd_debug_print("failed to decode json: $doc");
                return $channel_epg;
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return $channel_epg;
        }

        if (!empty($param_epg_root)) {
            foreach (explode('|', $param_epg_root) as $level) {
                $epg_root = trim($level, "[]");
                $ch_data = $ch_data[$epg_root];
            }
        }

        hd_debug_print("json epg root:    $param_epg_root", true);
        hd_debug_print("json start:       $param_epg_start", true);
        hd_debug_print("json title:       $param_epg_name", true);
        hd_debug_print("json desc:        $param_epg_desc", true);
        hd_debug_print("json icon:        $param_epg_icon", true);
        hd_debug_print("json time format: $param_epg_time_format", true);
        hd_debug_print("json timezone:    $param_epg_timezone", true);

        // collect all program that starts after day start and before day end
        $prev_start = 0;
        foreach ($ch_data as $entry) {
            $program_start = safe_get_value($entry, $param_epg_start);
            if (empty($program_start)) continue;

            if (!empty($param_epg_time_format)) {
                $time_format = str_replace(
                    array(Plugin_Macros::YEAR, Plugin_Macros::MONTH, Plugin_Macros::DAY, Plugin_Macros::HOUR, Plugin_Macros::MIN),
                    array('Y', 'm', 'd', 'H', 'i'),
                    $param_epg_time_format);

                $start = date_parse_from_format($time_format, $program_start);
                $program_start = gmmktime($start['hour'], $start['minute'], $start['second'], $start['month'], $start['day'], $start['year']);
            }

            if ($param_epg_timezone != 0) {
                $program_start -= $param_epg_timezone * 3600;
            }

            if ($prev_start !== 0) {
                $channel_epg[$prev_start][PluginTvEpgProgram::end_tm_sec] = $program_start;
            }
            $prev_start = $program_start;

            $channel_epg[$program_start][PluginTvEpgProgram::name] = unescape_entity_string(safe_get_value($entry, $param_epg_name, ''));
            $desc = unescape_entity_string(safe_get_value($entry, $param_epg_desc, ''));
            $icon = safe_get_value($entry, $param_epg_icon, '');

            if (empty($desc)) {
                $channel_epg[$program_start][PluginTvEpgProgram::description] = '';
            } else {
                $reformatted = self::reformat_description($desc, $icon);
                foreach ($reformatted as $key => $value) {
                    $channel_epg[$program_start][$key] = $value;
                }
            }

            if (!isset($channel_epg[$program_start][PluginTvEpgProgram::icon_url])) {
                $channel_epg[$program_start][PluginTvEpgProgram::icon_url] = $icon;
            }
        }

        if ($prev_start !== 0) {
            $channel_epg[$prev_start][PluginTvEpgProgram::end_tm_sec] = $prev_start + 3600; // fake end
        }

        ksort($channel_epg, SORT_NUMERIC);
        return $channel_epg;
    }

    /**
     * @param string $epg_url
     * @return array|false
     */
    protected static function get_channels_info($epg_url)
    {
        $channels_info_url = substr($epg_url, 0, strlen($epg_url) - strlen(basename($epg_url))) . 'channels_info.json';
        try {
            $ch_info_cache_file = get_data_path('/epg_cache/' . Hashed_Array::hash($epg_url) . ".cache");
            if (file_exists($ch_info_cache_file)) {
                $mtime = filemtime($ch_info_cache_file);
                $diff = time() - $mtime;
                if ($diff <= 3600 * 4) {
                    return parse_json_file($ch_info_cache_file);
                }
            }

            hd_debug_print("Fetching channels info from server: $channels_info_url");
            $ch_data = Curl_Wrapper::getInstance()->download_content($channels_info_url);
            if ($ch_data !== false) {
                file_put_contents($ch_info_cache_file, $ch_data);
            }

            if (empty($ch_data)) {
                throw new Exception('Empty document returned.');
            }

            return json_decode($ch_data, true);
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
        }
        return array();
    }
}

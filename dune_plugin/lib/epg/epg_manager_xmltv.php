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

require_once 'lib/hd.php';
require_once 'lib/hashed_array.php';
require_once 'lib/tr.php';

require_once 'epg_params.php';
require_once 'epg_indexer_classic.php';
require_once 'epg_indexer_sql.php';

class Epg_Manager_Xmltv
{
    /**
     * @var Default_Dune_Plugin
     */
    protected $plugin;

    /**
     * @var array
     */
    protected $delayed_epg = array();

    /**
     * @var int
     */
    protected $flags = 0;

    /**
     * @var Epg_Indexer
     */
    protected $indexer;

    /**
     * @param Default_Dune_Plugin|null $plugin
     */
    public function __construct($plugin = null)
    {
        $this->plugin = $plugin;
    }

    /**
     * Function to parse xmltv source in soparate process
     *
     * @param $config_file
     * @return bool
     */
    public function index_by_config($config_file)
    {
        global $LOG_FILE;

        if (!file_exists($config_file)) {
            HD::set_last_error("xmltv_last_error", "Config file for indexing not exist");
            return false;
        }

        $config = json_decode(file_get_contents($config_file));
        @unlink($config_file);
        if ($config === false) {
            HD::set_last_error("xmltv_last_error", "Invalid config file for indexing");
            return false;
        }

        if (empty($config->current_xmltv_source)) {
            return false;
        }

        $pid = getmypid();

        $LOG_FILE = $config->cache_dir . $config->current_xmltv_source . "_indexing.log";
        if (file_exists($LOG_FILE)) {
            @unlink($LOG_FILE);
        }
        date_default_timezone_set('UTC');

        set_debug_log($config->debug);

        hd_print("Script config");
        hd_print("Log: $LOG_FILE");
        hd_print("Process ID: $pid");
        hd_print("Active sources: " . pretty_json_format($config->active_xmltv_sources));
        hd_print("Current source: $config->current_xmltv_source");

        $this->init_indexer($config->cache_dir);
        $this->indexer->set_pid($pid);
        $this->indexer->set_active_sources(Hashed_Array::from_array($config->active_xmltv_sources));
        $this->indexer->index_all($config->current_xmltv_source);

        return true;
    }

    /**
     * @param string $cache_dir
     */
    public function init_indexer($cache_dir)
    {
        if (class_exists('SQLite3')) {
            $this->indexer = new Epg_Indexer_Sql();
        } else {
            $this->indexer = new Epg_Indexer_Classic();
        }

        $this->indexer->init($cache_dir);
        if ($this->plugin) {
            $flags = 0;
            $flags |= $this->plugin->get_bool_setting(PARAM_FAKE_EPG, false) ? EPG_FAKE_EPG : 0;
            $this->set_flags($flags);
            $this->indexer->set_active_sources($this->plugin->get_all_xmltv_sources());
        }
    }

    /**
     * @param int $flags
     * @return void
     */
    public function set_flags($flags)
    {
        $this->flags = $flags;
    }

    public function clear_epg_cache()
    {
        $this->indexer->clear_all_epg_files();
    }

    /**
     * Try to load epg from cached file
     *
     * @param Channel $channel
     * @param int $day_start_ts
     * @return array
     */
    public function get_day_epg_items(Channel $channel, $day_start_ts)
    {
        $active_sources = $this->plugin->get_all_xmltv_sources();
        $any_lock = $this->indexer->is_any_index_locked();
        $day_epg = array();
        $ext_epg = $this->plugin->is_ext_epg_enabled();
        if (!$active_sources->size()) {
            return array($day_start_ts => array(
                Epg_Params::EPG_END => $day_start_ts + 86400,
                Epg_Params::EPG_NAME => TR::load('epg_not_exist'),
                Epg_Params::EPG_DESC => TR::load('epg_not_set'),
            ));
        }

        foreach($active_sources as $hash => $source) {
            if ($this->indexer->is_index_locked($hash)) {
                hd_debug_print("EPG $source->url still indexing, append to delayed queue channel id: " . $channel->get_id());
                $this->delayed_epg[] = $channel->get_id();
                continue;
            }

            if ($source === null || (int)$source->ttl === -2) {
                continue;
            }

            // filter out epg only for selected day
            $day_end_ts = $day_start_ts + 86400;
            $date_start_l = format_datetime("Y-m-d H:i", $day_start_ts);
            $date_end_l = format_datetime("Y-m-d H:i", $day_end_ts);
            hd_debug_print("Fetch entries for from: $date_start_l ($day_start_ts) to: $date_end_l ($day_end_ts)");

            try {
                $positions = $this->indexer->load_program_index($hash, $channel);
                if (!empty($positions)) {
                    $cached_file = $this->indexer->get_cache_filename($hash);
                    if (!file_exists($cached_file)) {
                        throw new Exception("cache file $cached_file not exist");
                    }

                    $handle = fopen($cached_file, 'rb');
                    if ($handle) {
                        foreach ($positions as $pos) {
                            fseek($handle, $pos['start']);
                            $length = $pos['end'] - $pos['start'];
                            if ($length <= 0) continue;

                            $xml_str = "<tv>" . fread($handle, $pos['end'] - $pos['start']) . "</tv>";

                            $xml_node = new DOMDocument();
                            $res = $xml_node->loadXML($xml_str);
                            if ($res === false) {
                                throw new Exception("Exception in line: $xml_str");
                            }

                            foreach ($xml_node->getElementsByTagName('programme') as $tag) {
                                $program_start = strtotime($tag->getAttribute('start'));
                                $program_end = strtotime($tag->getAttribute('stop'));
                                if ($program_start < $day_start_ts && $program_end < $day_start_ts) continue;
                                if ($program_start >= $day_end_ts) break;

                                $day_epg[$program_start][Epg_Params::EPG_END] = $program_end;
                                $day_epg[$program_start][Epg_Params::EPG_NAME] = self::get_node_value($tag, 'title');
                                $day_epg[$program_start][Epg_Params::EPG_DESC] = HD::unescape_entity_string(self::get_node_value($tag, 'desc'));
                                $day_epg[$program_start][Epg_Params::EPG_ICON] = self::get_node_attribute($tag, 'icon', 'src');

                                if (!$ext_epg) continue;

                                $day_epg[$program_start][PluginTvExtEpgProgram::sub_title] = self::get_node_value($tag, 'sub-title');
                                $day_epg[$program_start][PluginTvExtEpgProgram::main_category] = self::get_node_value($tag, 'category');
                                $day_epg[$program_start][PluginTvExtEpgProgram::year] = self::get_node_value($tag, 'date');
                                $day_epg[$program_start][PluginTvExtEpgProgram::country] = self::get_node_value($tag, 'country');
                                foreach ($tag->getElementsByTagName('credits') as $sub_tag) {
                                    $day_epg[$program_start][PluginTvExtEpgProgram::director] = self::get_node_value($sub_tag, 'director');
                                    $day_epg[$program_start][PluginTvExtEpgProgram::producer] = self::get_node_value($sub_tag, 'producer');
                                    $day_epg[$program_start][PluginTvExtEpgProgram::actor] = self::get_node_value($sub_tag, 'actor');
                                    $day_epg[$program_start][PluginTvExtEpgProgram::presenter] = self::get_node_value($sub_tag, 'presenter'); //Ведущий
                                    $day_epg[$program_start][PluginTvExtEpgProgram::writer] = self::get_node_value($sub_tag, 'writer');
                                    $day_epg[$program_start][PluginTvExtEpgProgram::editor] = self::get_node_value($sub_tag, 'editor');
                                    $day_epg[$program_start][PluginTvExtEpgProgram::composer] = self::get_node_value($sub_tag, 'composer');
                                }
                                foreach ($tag->getElementsByTagName('image') as $sub_tag) {
                                    if (!empty($sub_tag->nodeValue)) {
                                        $day_epg[$program_start][PluginTvExtEpgProgram::icon_urls][] = $sub_tag->nodeValue;
                                    }
                                }
                            }
                        }

                        fclose($handle);

                        if (!empty($day_epg)) break;
                    }
                }
            } catch (Exception $ex) {
                print_backtrace_exception($ex);
            }
        }

        if (empty($day_epg)) {
            if ($any_lock !== false) {
                $this->delayed_epg = array_unique($this->delayed_epg);
                return array($day_start_ts => array(
                    Epg_Params::EPG_END => $day_start_ts + 86400,
                    Epg_Params::EPG_NAME => TR::load('epg_not_ready'),
                    Epg_Params::EPG_DESC => TR::load('epg_not_ready_desc'),
                ));
            }
            return $this->getFakeEpg($channel, $day_start_ts, $day_epg);
        }

        ksort($day_epg);

        return $day_epg;
    }

    /**
     * @param string $epg_source
     * @param string $epg_id
     * @param string $channel_id
     * @param int $day_start_ts
     * @return string
     */
    public function get_egp_url($epg_source, $epg_id, $channel_id, $day_start_ts)
    {
        return '';
    }

    /**
     * Import indexing log to plugin logs
     *
     * @return bool true if import successful and no other active locks, false if any active source is locked
     */
    public function import_indexing_log()
    {
        $has_locks = false;
        foreach ($this->indexer->get_active_sources()->get_keys() as $hash) {
            if ($this->indexer->is_index_locked($hash)) {
                $has_locks = true;
                continue;
            }

            $index_log = $this->indexer->get_cache_dir() . "{$hash}_indexing.log";
            if (file_exists($index_log)) {
                hd_debug_print("Read epg indexing log $index_log...");
                hd_debug_print_separator();
                $logfile = @file_get_contents($index_log);
                foreach (explode(PHP_EOL, $logfile) as $l) {
                    hd_print(preg_replace("|^\[.+\]\s(.*)$|", "$1", rtrim($l)));
                }
                hd_debug_print_separator();
                hd_debug_print("Read finished");
                @unlink($index_log);
            }
        }

        return !$has_locks;
    }

    /**
     * @return Epg_Indexer
     */
    public function get_indexer()
    {
        return $this->indexer;
    }

    /**
     * returns list of requested epg when indexing in process
     *
     * @return array
     */
    public function get_delayed_epg()
    {
        return $this->delayed_epg;
    }

    /**
     * clear all delayed epg
     */
    public function clear_delayed_epg()
    {
        $this->delayed_epg = array();
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// protected methods

    /**
     * @param Channel $channel
     * @param int $day_start_ts
     * @param array $day_epg
     * @return array
     */
    protected function getFakeEpg(Channel $channel, $day_start_ts, $day_epg)
    {
        if (($this->flags & EPG_FAKE_EPG) && $channel->get_archive() !== 0) {
            hd_debug_print("Create fake data for non existing EPG data");
            for ($start = $day_start_ts, $n = 1; $start <= $day_start_ts + 86400; $start += 3600, $n++) {
                $day_epg[$start][Epg_Params::EPG_END] = $start + 3600;
                $day_epg[$start][Epg_Params::EPG_NAME] = TR::load('fake_epg_program') . " $n";
                $day_epg[$start][Epg_Params::EPG_DESC] = '';
            }
        } else {
            hd_debug_print("No EPG for channel: {$channel->get_id()}");
        }

        return $day_epg;
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// static methods

    protected static function get_node_value($node, $name)
    {
        $value = '';
        foreach ($node->getElementsByTagName($name) as $element) {
            if (!empty($element->nodeValue)) {
                $value = $element->nodeValue;
                break;
            }
        }

        return $value;
    }

    protected static function get_node_attribute($node, $name, $attribute)
    {
        $value = '';
        foreach ($node->getElementsByTagName($name) as $element) {
            $value = $element->getAttribute($attribute);
            break;
        }

        return $value;
    }
}

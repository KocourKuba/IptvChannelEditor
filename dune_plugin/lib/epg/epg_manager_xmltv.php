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

require_once 'epg_indexer_classic.php';
require_once 'epg_indexer_sql.php';
require_once 'lib/epg/ext_epg_program.php';

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
     * @var bool
     */
    protected static $ext_epg_enabled;

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
        self::$ext_epg_enabled = $this->plugin->is_ext_epg_enabled();
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
        safe_unlink($config_file);
        if ($config === false) {
            HD::set_last_error("xmltv_last_error", "Invalid config file for indexing");
            return false;
        }

        if (empty($config->current_xmltv_source)) {
            return false;
        }

        $pid = getmypid();

        $LOG_FILE = $config->cache_dir . $config->current_xmltv_source . "_indexing.log";
        safe_unlink($LOG_FILE);
        date_default_timezone_set('UTC');

        set_debug_log($config->debug);

        hd_print("Script config");
        hd_print("Log: $LOG_FILE");
        hd_print("Process ID: $pid");
        hd_print("Active sources: " . json_format_unescaped($config->active_xmltv_sources));
        hd_print("Current source: $config->current_xmltv_source");

        $this->init_indexer($config->cache_dir);
        $this->indexer->get_curl_wrapper()->set_connect_timeout($config->connect_timeout);
        $this->indexer->get_curl_wrapper()->set_download_timeout($config->download_timeout);
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
        if (!$active_sources->size()) {
            return array($day_start_ts => array(
                PluginTvEpgProgram::end_tm_sec => $day_start_ts + 86400,
                PluginTvEpgProgram::name => TR::load('epg_not_exist'),
                PluginTvEpgProgram::description => TR::load('epg_not_set'),
            ));
        }

        $channel_id = $channel->get_id();
        foreach($active_sources as $hash => $source) {
            if ($this->indexer->is_index_locked($hash)) {
                hd_debug_print("EPG $source->url still indexing, append to delayed queue channel id: " . $channel_id);
                $this->delayed_epg[] = $channel_id;
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

                    $update_ext_epg = function($tag_name, $node_name, $tag, &$item) {
                        $value = Epg_Manager_Xmltv::get_node_value($tag, $node_name);
                        if (!empty($value)) {
                            $item[$tag_name] = $value;
                        }
                    };

                    $collect_ext_epg = function($tag_name, $node_name, $tag, &$item) {
                        $value = Epg_Manager_Xmltv::get_node_values($tag, $node_name);
                        if (!empty($value)) {
                            $item[$tag_name] = $value;
                        }
                    };

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

                                $desc = HD::unescape_entity_string(self::get_node_value($tag, 'desc'));
                                $icon = self::get_node_attribute($tag, 'icon', 'src');
                                $day_epg[$program_start][PluginTvEpgProgram::end_tm_sec] = $program_end;
                                $day_epg[$program_start][PluginTvEpgProgram::name] = self::get_node_value($tag, 'title');

                                if (empty($desc)) {
                                    $day_epg[$program_start][PluginTvEpgProgram::description] = '';
                                } else {
                                    $reformatted = self::reformat_description($desc, $icon);
                                    foreach ($reformatted as $key => $value) {
                                        $day_epg[$program_start][$key] = $value;
                                    }
                                }

                                if (!self::$ext_epg_enabled) continue;

                                $update_ext_epg(PluginTvExtEpgProgram::sub_title, 'sub-title', $tag, $day_epg[$program_start]);
                                $update_ext_epg(PluginTvExtEpgProgram::main_category, 'category', $tag, $day_epg[$program_start]);
                                $update_ext_epg(PluginTvExtEpgProgram::year, 'date', $tag, $day_epg[$program_start]);
                                $update_ext_epg(PluginTvExtEpgProgram::country, 'country', $tag, $day_epg[$program_start]);

                                $collect_ext_epg(PluginTvExtEpgProgram::icons, 'image', $tag, $day_epg[$program_start]);
                                foreach ($tag->getElementsByTagName('credits') as $sub_tag) {
                                    $collect_ext_epg(PluginTvExtEpgProgram::director, 'director', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::producer, 'producer', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::actor, 'actor', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::presenter, 'presenter', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::writer, 'writer', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::editor, 'editor', $sub_tag, $day_epg[$program_start]);
                                    $collect_ext_epg(PluginTvExtEpgProgram::composer, 'composer', $sub_tag, $day_epg[$program_start]);
                                }
                            }
                        }

                        fclose($handle);

                        if (!empty($day_epg)) break;
                    }
                }
            } catch (Exception $ex) {
                $day_epg = array();
                print_backtrace_exception($ex);
            }
        }

        if (empty($day_epg)) {
            if ($any_lock !== false) {
                $this->delayed_epg = array_unique($this->delayed_epg);
                $day_epg = array($day_start_ts => array(
                    PluginTvEpgProgram::end_tm_sec => $day_start_ts + 86400,
                    PluginTvEpgProgram::name => TR::load('epg_not_ready'),
                    PluginTvEpgProgram::description => TR::load('epg_not_ready_desc'),
                ));
            } else {
                $day_epg = $this->getFakeEpg($channel, $day_start_ts);
            }
        } else {
            hd_debug_print("Store day epg to memory cache");
            ksort($day_epg);
        }

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

    public static function reformat_description($raw_descr, $icon)
    {
        $total = array();

        $find_chunks = function ($chunks, $raw_descr) use (&$total) {
            foreach ($chunks as $key => $pattern) {
                $m = preg_split($pattern, $raw_descr, 0, PREG_SPLIT_DELIM_CAPTURE);
                if (!isset($m[1])) continue;

                $total[$key] = trim($m[1]);
                $raw_descr = preg_replace($pattern, '', $raw_descr);
            }

            if (preg_match("/\.\.\.(.*?) \|/", $raw_descr, $m)) {
                $raw_descr = $m[1];
            }
            return trim($raw_descr, ", \n\r\t\v\0");
        };

        if (strpos($icon, "media.24h.tv") !== false) {
            // 24tv
            $icon = $icon . "?cover=true&w=320&h=180&crop=true";

            $chunks = array(
                "year" => "/Год: (\d\d\d\d)/",
                "country" => "/Страна: (.*?)(\.|,|\s)/u",
                "genre" => "/Жанр: (.*?)[\.\n]/u",
                "imdb_rating" => "/(?:Рейтинг:?\s*)?IMDb\s*\[(.*?)\]/u",
                "kp_rating" => "/КиноПоиск\[(.*?)\]/u",
                "director" => "/Режисс[её]ры?: (.*?)(\.|\n)/u",
                "actor" => "/В [Рр]олях: (.*?)$/u",
            );

            $raw_descr = $find_chunks($chunks, $raw_descr);
        } else if (strpos($icon, "resizer.mail.ru") !== false || strpos($icon, "kinopoisk-ru") !== false) {
            // mail.ru
            $chunks = array(
                "year" => "/Год: (\d\d\d\d)/",
                "country" => "/Страна: (.*?)(\.|,|\s)/u",
                "genre" => "/Жанр: (.*?)[\.\n]/u",
                "imdb_rating" => "/(?:Рейтинг:?\s*)?IMDb\s*\[(.*?)\]/u",
                "kp_rating" => "/Рейтинг Кинопоиска \[(.*?)\]\.?/u",
                "kinomail_rating" => "/(?:Рейтинг )?KinoMail \[(.*?)\]\./u",
                "director" => "/Режисс[её]ры?: (.*?)(\.|\n)/u",
                "actor" => "/В [Рр]олях: (.*?)\.?\n/u",
                "writer" => "/Сценарий: (.*?)[\.\n]/u",
                "editor" => "/Операторы?: (.*?)\.?\n/u",
                "composer" => "/Композиторы?: (.*?)\.?\n/u",
                "rating" => "/Рейтинг: \((.*?)\)/u",
                "producer" => "/Продюсеры?: (.*?)\.?\n/u",
                "budget" => "/Бюджет: (.*?)\./u",
                "original_name" => "/Оригинальное название: (.*?)\.?$/"
            );

            $raw_descr = $find_chunks($chunks, $raw_descr);
        } else {
            $common_chunks = array(
                "year" => "/Год: (\d\d\d\d)/",
                "country" => "/Страна: (.*?)(\.|,|\s)/u",
                "genre" => "/Жанр: (.*?)[\.\n]/u",
                "imdb_rating" => "/(?:Рейтинг:?\s*)?IMDb\s*\[(.*?)\]/u",
                "director" => "/Режисс[её]ры?: (.*?)(\.|\n)/u",
                "actor" => "/В [Рр]олях: (.*)\.?\n?/u",
            );
            $raw_descr = $find_chunks($common_chunks, $raw_descr);
        }

        $raw_descr = preg_replace("/Нет описания/", '', $raw_descr);
        $raw_descr = preg_replace("/Описание отсутствует/", '', $raw_descr);
        $raw_descr = preg_replace('/Смотреть онлайн.*вас время!/', '', $raw_descr);
        $raw_descr = preg_replace('/Смотрите.*на Wink/', '', $raw_descr);
        $raw_descr = preg_replace('/смотрите.*на Wink/', '', $raw_descr);
        $raw_descr = preg_replace('/Донат.*/s', '', $raw_descr);
        $raw_descr = preg_replace('/,\s{2}\((.*?)\)/', '', $raw_descr);
        $raw_descr = str_replace(array('“', '”'), '', $raw_descr);
        $raw_descr = str_replace(array("\n\n", '<br>', "<'>br>"), "\n", $raw_descr);
        $raw_descr = trim($raw_descr," ,\n\r\t\v\0");
        //$raw_descr = mb_substr($raw_descr, 0, 1670);

        $result = array();
        if (self::$ext_epg_enabled) {
            if (isset($total['genre']))
                $result[PluginTvExtEpgProgram::main_category] = $total['genre'];
            if (isset($total['year']))
                $result[PluginTvExtEpgProgram::year] = $total['year'];
            if (isset($total['country']))
                $result[PluginTvExtEpgProgram::country] = $total['country'];
            if (isset($total['director']))
                $result[PluginTvExtEpgProgram::director] = $total['director'];
            if (isset($total['actor']))
                $result[PluginTvExtEpgProgram::actor] = $total['actor'];
            if (isset($total['imdb_rating']))
                $result[PluginTvExtEpgProgram::imdb_rating] = $total['imdb_rating'];
            if (isset($total['kp_rating']))
                $result[PluginTvExtEpgProgram::kp_rating] = $total['kp_rating'];
            if (isset($total['kinomail_rating']))
                $result[PluginTvExtEpgProgram::km_rating] = $total['kinomail_rating'];
            if (isset($total['rating'])) {
                if (isset($total['kp_rating']))
                    $result[PluginTvExtEpgProgram::imdb_rating] = $total['kp_rating'];
                else if (isset($total['km_rating']))
                    $result[PluginTvExtEpgProgram::imdb_rating] = $total['km_rating'];
            }
            if (isset($total["writer"]))
                $result[PluginTvExtEpgProgram::writer] = $total['writer'];
            if (isset($total["editor"]))
                $result[PluginTvExtEpgProgram::editor] = $total['editor'];
            if (isset($total["composer"]))
                $result[PluginTvExtEpgProgram::composer] = $total['composer'];
            if (isset($total["presenter"]))
                $result[PluginTvExtEpgProgram::presenter] = $total['presenter']; //Ведущий

            $result[PluginTvExtEpgProgram::main_icon] = $icon;
        }

        $result[PluginTvEpgProgram::description] = $raw_descr;
        $result[PluginTvEpgProgram::icon_url] = $icon;

        return $result;
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
            }
            safe_unlink($index_log);
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
     * @return array
     */
    protected function getFakeEpg(Channel $channel, $day_start_ts)
    {
        $day_epg = array();
        if (($this->flags & EPG_FAKE_EPG) && $channel->get_archive() !== 0) {
            hd_debug_print("Create fake data for non existing EPG data");
            for ($start = $day_start_ts, $n = 1; $start <= $day_start_ts + 86400; $start += 3600, $n++) {
                $day_epg[$start][PluginTvEpgProgram::end_tm_sec] = $start + 3600;
                $day_epg[$start][PluginTvEpgProgram::name] = TR::load('fake_epg_program') . " $n";
                $day_epg[$start][PluginTvEpgProgram::description] = '';
            }
        } else {
            hd_debug_print("No EPG for channel: {$channel->get_id()}");
        }

        return $day_epg;
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// static methods

    /**
     * @param DOMElement $node
     * @param string $name
     * @return string
     */
    public static function get_node_value($node, $name)
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

    /**
     * @param DOMElement $node
     * @param string $name
     * @return array
     */
    public static function get_node_values($node, $name)
    {
        $values = array();
        foreach ($node->getElementsByTagName($name) as $element) {
            if (!empty($element->nodeValue)) {
                $values[] = $element->nodeValue;
            }
        }

        return $values;
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

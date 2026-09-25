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

require_once 'epg_indexer.php';

class Epg_Indexer_Classic extends Epg_Indexer
{
    /**
     * contains indexes for xmltv file (hash)
     * @var array
     */
    protected $xmltv_indexes;

    /**
     * @inheritDoc
     * @override
     */
    public function init($cache_dir)
    {
        parent::init($cache_dir);

        $this->index_ext = '.index';
    }

    /**
     * @inheritDoc
     * @override
     */
    public function get_epg_id($hash, $channel)
    {
        if (empty($this->xmltv_indexes[$hash][self::INDEX_CHANNELS])) {
            $this->perf->reset('start');
            $index_file = $this->get_index_name(self::INDEX_CHANNELS, $hash);
            hd_debug_print("load channels index $$index_file");
            $data = parse_json_file($index_file);
            if ($data === false) {
                hd_debug_print("load channels index failed '$index_file'");
                return '';
            }

            $this->perf->setLabel('end_load');
            $report = $this->perf->getFullReport();
            hd_debug_print("Load time: {$report[Perf_Collector::TIME]} secs");
            hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");

            $this->xmltv_indexes[$hash][self::INDEX_CHANNELS] = $data;
        }

        // try found channel_id by epg_id
        $epg_ids = $channel->get_epg_ids();
        $channels = $this->xmltv_indexes[$hash][self::INDEX_CHANNELS];
        foreach ($epg_ids as $epg_id) {
            $epg_id_lower = mb_convert_case($epg_id, MB_CASE_LOWER, "UTF-8");
            if (array_key_exists($epg_id_lower, $channels)) {
                return $channels[$epg_id_lower];
            }
        }

        return '';
    }

    /**
     * @inheritDoc
     * @override
     */
    public function load_program_index($hash, $channel)
    {
        try {
            $this->perf->reset('start');

            $channel_id = $this->get_epg_id($hash, $channel);

            if (empty($channel_id)) {
                throw new Exception("index positions for epg '{$channel->get_title()}' is not exist");
            }

            $this->perf->setLabel('fetch');
            if (empty($this->xmltv_indexes[$hash][self::INDEX_ENTRIES])) {
                $index_file = $this->get_index_name(self::INDEX_ENTRIES, $hash);
                hd_debug_print("load positions index $index_file");
                $data = parse_json_file($index_file);
                if ($data === false) {
                    throw new Exception("load positions index failed '$index_file'");
                }
                $this->xmltv_indexes[$hash][self::INDEX_ENTRIES] = $data;
                $this->perf->setLabel('end_load');

                $report = $this->perf->getFullReport();
                hd_debug_print("Load time: {$report[Perf_Collector::TIME]} secs");
                hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
            }

            $positions = $this->xmltv_indexes[$hash][self::INDEX_ENTRIES];
            if (!isset($positions[$channel_id])) {
                throw new Exception("index positions for epg $channel_id is not exist");
            }

            $this->perf->setLabel('end');
            $report = $this->perf->getFullReport('fetch');

            hd_debug_print("Fetch positions "
                . count($positions[$channel_id])
                . " for '$channel_id' by channel: '{$channel->get_title()}' ({$channel->get_id()}) done in: {$report[Perf_Collector::TIME]} secs");

            return $positions[$channel_id];
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
        }

        return array();
    }

    /**
     * @inheritDoc
     * @override
     */
    public function index_xmltv_channels($hash)
    {
        if ($this->is_index_locked($hash)) {
            hd_debug_print("File is indexing or downloading, skipped");
            return;
        }

        $this->perf->reset('start');

        $channels_file = $this->get_index_name(self::INDEX_CHANNELS, $hash);

        if (!isset($this->xmltv_indexes[$hash][self::INDEX_CHANNELS])
            && $this->is_all_indexes_valid(array(self::INDEX_CHANNELS), $hash)) {
            hd_debug_print("Load cache channels and picons index: $channels_file");
            $data = parse_json_file($channels_file);
            $success = true;
            if ($data !== false) {
                $this->xmltv_indexes[$hash][self::INDEX_CHANNELS] = $data;
            } else {
                hd_debug_print("load positions index failed '$channels_file'");
                $success = false;
            }

            if ($success) {
                $this->perf->setLabel('end');
                $report = $this->perf->getFullReport();
                hd_debug_print("ParseFile: {$report[Perf_Collector::TIME]} secs");
                hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
                hd_debug_print_separator();
                return;
            }
        }

        $this->xmltv_indexes[$hash][self::INDEX_CHANNELS] = array();

        $this->perf->setLabel('reindex');

        try {
            $this->set_index_locked($hash, true);

            hd_debug_print_separator();
            hd_debug_print("Start reindex: $channels_file");

            $channels = array();
            $picons = 0;
            $store = function ($batch) use (&$channels, &$picons) {
                foreach ($batch as $channel) {
                    $channel_id = $channel['id'];
                    $channels[mb_convert_case($channel_id, MB_CASE_LOWER, "UTF-8")] = $channel_id;
                    foreach ($channel['aliases'] as $alias) {
                        $channels[mb_convert_case($alias, MB_CASE_LOWER, "UTF-8")] = $channel_id;
                    }
                    if (!empty($channel['picon'])) {
                        $picons++;
                    }
                }
            };

            $file = $this->open_xmltv_file($hash);
            self::scan_xmltv_channels($file, $store);
            fclose($file);

            store_to_json_file($channels_file, $channels);

            $this->xmltv_indexes[$hash][self::INDEX_CHANNELS] = $channels;

            $this->perf->setLabel('end');
            $report = $this->perf->getFullReport();
            hd_debug_print("Total entries id's: " . count($channels));
            hd_debug_print("Total known picons: $picons");
            hd_debug_print("Reindexing EPG channels done: {$report[Perf_Collector::TIME]} secs");
            hd_debug_print("Storage space in cache dir after reindexing: " . HD::get_storage_size($this->cache_dir));
            hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
        } catch (Exception $ex) {
            hd_debug_print("Reindexing EPG channels failed");
            print_backtrace_exception($ex);
        }

        $this->set_index_locked($hash, false);
        hd_debug_print_separator();
    }

    /**
     * @inheritDoc
     * @override
     */
    public function index_xmltv_positions($hash)
    {
        hd_debug_print("Indexing positions for: $hash", true);
        if ($this->is_index_locked($hash)) {
            hd_debug_print("File is indexing now, skipped");
            return;
        }

        $this->perf->reset('start');

        $positions_file = $this->get_index_name(self::INDEX_ENTRIES, $hash);
        if (empty($this->xmltv_indexes[$hash][self::INDEX_ENTRIES]) && $this->is_all_indexes_valid(array(self::INDEX_ENTRIES), $hash)) {
            hd_debug_print("Try load cache program index: $positions_file");
            $success = true;
            $data = parse_json_file($positions_file);
            if ($data !== false) {
                $this->xmltv_indexes[$hash][self::INDEX_ENTRIES] = $data;
            } else {
                hd_debug_print("load positions index failed '$positions_file'");
                $success = false;
            }

            $this->perf->setLabel('end');
            $report = $this->perf->getFullReport();

            if ($success) {
                hd_debug_print("Load time: {$report[Perf_Collector::TIME]} secs");
                hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
                return;
            }
        }

        try {

            hd_debug_print("Start reindex: $positions_file");

            $this->perf->setLabel('reindex');

            $this->remove_index(self::INDEX_ENTRIES, $hash);

            $this->set_index_locked($hash, true);
            $file = $this->open_xmltv_file($hash);

            $xmltv_index = array();
            $store = function ($channel_id, $start, $end) use (&$xmltv_index) {
                $xmltv_index[$channel_id][] = array('start' => $start, 'end' => $end);
            };
            self::scan_xmltv_positions($file, $store);
            fclose($file);

            if (!empty($xmltv_index)) {
                hd_debug_print("Save index: $positions_file", true);
                store_to_json_file($this->get_index_name(self::INDEX_ENTRIES, $hash), $xmltv_index);
                $this->xmltv_indexes[$hash][self::INDEX_ENTRIES] = $xmltv_index;
            }

            $this->perf->setLabel('end');
            $report = $this->perf->getFullReport();
            hd_debug_print("Total unique epg id's indexed: " . count($xmltv_index));
            hd_debug_print("Reindexing EPG program done: {$report[Perf_Collector::TIME]} secs");
            hd_debug_print("Storage space in cache dir after reindexing: " . HD::get_storage_size($this->cache_dir));
            hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
        } catch (Exception $ex) {
            hd_debug_print("Reindexing EPG positions failed");
            print_backtrace_exception($ex);
        }

        $this->set_index_locked($hash, false);
        hd_debug_print_separator();
    }

    /**
     * @inheritDoc
     * @override
     */
    public function remove_index($name, $hash)
    {
        $name = $this->get_index_name($name, $hash);
        safe_unlink($name);
        return true;
    }

    /**
     * @inheritDoc
     * @override
     */
    public function remove_all_indexes($hash)
    {
        foreach (array(self::INDEX_CHANNELS, self::INDEX_ENTRIES) as $name) {
            $filename = $this->get_index_name($name, $hash);
            safe_unlink($filename);
        }
    }

    /**
     * @inheritDoc
     * @override
     */
    public function get_indexes_info($hash)
    {
        $result = array(self::INDEX_CHANNELS => -1, self::INDEX_ENTRIES => -1);
        foreach ($result as $index => $name) {
            if (isset($this->xmltv_indexes[$hash][$index])) {
                $result[$index] = count($this->xmltv_indexes[$hash][$index]);
                continue;
            }

            $filename = $this->get_cache_filename($hash, "_$index$this->index_ext");
            if (file_exists($filename) && filesize($filename) !== 0) {
                $data = parse_json_file($filename);
                if ($data !== false) {
                    $this->xmltv_indexes[$hash][$index] = $data;
                    $result[$index] = count($data);
                } else {
                    hd_debug_print("Failed to load index: $filename");
                }
            }
        }

        return $result;
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// protected methods

    /**
     * @param string $name
     * @return string
     */
    protected function get_index_name($name, $hash)
    {
        return $this->get_cache_filename($hash, "_$name$this->index_ext");
    }

    /**
     * @inheritDoc
     * @override
     */
    protected function is_all_indexes_valid($names, $hash)
    {
        foreach ($names as $name) {
            $name = $this->get_index_name($name, $hash);
            if (!file_exists($name) || filesize($name) === 0) {
                return false;
            }
        }

        return true;
    }

    /**
     * @inheritDoc
     * @override
     */
    protected function clear_memory_index($id = '')
    {
        hd_debug_print("clear legacy index");

        if (empty($id)) {
            $this->xmltv_indexes = array();
        } else {
            unset($this->xmltv_indexes[$id]);
        }
    }
}

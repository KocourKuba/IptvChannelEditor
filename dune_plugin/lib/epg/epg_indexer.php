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

require_once 'epg_indexer_interface.php';

require_once 'lib/hd.php';
require_once 'lib/hashed_array.php';
require_once 'lib/curl_wrapper.php';
require_once 'lib/perf_collector.php';

abstract class Epg_Indexer implements Epg_Indexer_Interface
{
    const STREAM_CHUNK = 131072; // 128Kb
    const INDEX_CHANNELS = 'epg_channels';
    const INDEX_ENTRIES = 'epg_entries';

    /**
     * Size of the chunk read at once while scanning the xmltv file for <programme> open tags.
     * Big enough that one fread() covers many elements, small enough to stay well inside the
     * device memory budget (the block is held as a single PHP string).
     */
    const INDEX_BLOCK_SIZE = 65536;

    /**
     * Number of <programme> open tags a block must hold before the regex jump is used for the
     * next one. Below this the block is mostly element bodies and there is not enough per-tag
     * work to pay for the jump.
     */
    const SPARSE_BLOCK_TAGS = 16;

    /**
     * Number of channel changes inside one block above which the regex jump is dropped for the
     * next one. Every change compiles a new pattern, so a source that interleaves the programmes
     * of different channels is scanned faster by walking the open tags.
     */
    const DENSE_BLOCK_TRANSITIONS = 16;

    /**
     * While the regex jump is in use the open tags are not counted, so every this many blocks one
     * block is walked tag by tag to re-measure how dense they are.
     */
    const RESAMPLE_BLOCKS = 128;

    /**
     * Size of the chunk read at once while collecting <channel> elements.
     */
    const CHANNELS_BLOCK_SIZE = 65536;

    /**
     * How many <channel> elements are parsed with a single DOMDocument. Parsing them one by one
     * sets up a libxml parser per channel, which dominates the pass on sources with thousands of
     * channels.
     */
    const CHANNELS_BATCH_SIZE = 64;

    /**
     * A <channel> element that does not close within this many bytes is treated as malformed
     * and dropped, so a broken source cannot pull the whole file into the buffer.
     */
    const MAX_CHANNEL_ELEMENT_SIZE = 1048576;

    /**
     * path where cache is stored
     * @var string
     */
    protected $cache_dir;

    /**
     * @var string
     */
    protected $index_ext;

    /**
     * @var Hashed_Array<string, Cache_Parameters>
     */
    protected $active_sources;

    /**
     * @var Curl_Wrapper
     */
    protected $curl_wrapper;

    /**
     * @var int
     */
    protected $pid = 0;

    /**
     * @var Perf_Collector
     */
    protected $perf;

    public function __construct()
    {
        $this->curl_wrapper = new Curl_Wrapper();
        $this->perf = new Perf_Collector();
        $this->active_sources = new Hashed_Array();
    }

    /**
     * @param string $cache_dir
     */
    public function init($cache_dir)
    {
        $this->cache_dir = get_slash_trailed_path($cache_dir);
        create_path($this->cache_dir);

        hd_debug_print("Indexer engine: " . get_class($this));
        hd_debug_print("Cache dir: $this->cache_dir");
        hd_debug_print("Storage space in cache dir: " . HD::get_storage_size($this->cache_dir));
    }

    /**
     * @param int $pid
     * @return void
     */
    public function set_pid($pid)
    {
        $this->pid = $pid;
    }

    /**
     * @return int
     */
    public function get_pid()
    {
        return $this->pid;
    }

    /**
     * @return Curl_Wrapper
     */
    public function get_curl_wrapper()
    {
        return $this->curl_wrapper;
    }

    /**
     * @param Hashed_Array<string, Cache_Parameters> $urls
     * @return void
     */
    public function set_active_sources($urls)
    {
        $this->active_sources = $urls;
        if ($this->active_sources->size() === 0) {
            hd_debug_print("No XMLTV source selected");
        } else {
            hd_debug_print("XMLTV sources selected: $this->active_sources");
        }
    }

    /**
     * @return Hashed_Array<string, Cache_Parameters>
     */
    public function get_active_sources()
    {
        return $this->active_sources;
    }

    /**
     * @return string
     */
    public function get_cache_dir()
    {
        return $this->cache_dir;
    }

    /**
     * Indexing xmltv file to make channel to display-name map and collect picons for channels.
     * This function called from script only and plugin not available in this call
     *
     * @param string $hash
     * @return void
     */
    public function index_all($hash)
    {
        /** @var Cache_Parameters $source */
        $source = $this->active_sources->get($hash);
        if ($source === null || empty($source->url)) {
            hd_debug_print("Source not found or XMTLV EPG url not set");
            return;
        }

        if ((int)$source->ttl === -2) {
            hd_debug_print("Source $hash disabled: $source->url");
            return;
        }

        hd_debug_print("Processing source: " . json_format_unescaped($source), true);

        $res = $this->is_xmltv_cache_valid($hash, $source);
        hd_debug_print("cache valid status: $res", true);
        switch ($res) {
            case 1:
                // downloaded xmltv file not exists or expired
                hd_debug_print("Download and indexing xmltv source: $source->url", true);
                $this->remove_all_indexes($hash);
                if ($this->download_xmltv_source($hash, $source) === 1) {
                    $this->index_xmltv_channels($hash);
                    $this->index_xmltv_positions($hash);
                }
                break;
            case 2:
                // downloaded xmltv file exists, not expired but indexes for positions not valid
                hd_debug_print("Indexing xmltv positions: $source->url", true);
                $this->remove_index(self::INDEX_ENTRIES, $hash);
                $this->index_xmltv_positions($hash);
                break;
            case 3:
                // downloaded xmltv file exists, not expired but indexes for channels, picons and positions not valid
                hd_debug_print("Indexing xmltv source: $source->url", true);
                $this->remove_all_indexes($hash);
                $this->index_xmltv_channels($hash);
                $this->index_xmltv_positions($hash);
                break;
            default:
                break;
        }
    }

    /**
     * Checks if xmltv source cached and not expired.
     * if xmltv url not set return -1 and set_last_error contains error message
     * if downloaded xmltv file exists and all indexes are present return 0
     * if downloaded xmltv file not exists or expired return 1
     * if downloaded xmltv file exists, not expired and indexes for channels and icons exists return 2
     * if downloaded xmltv file exists, not expired but all indexes not exists return 3
     *
     * @param string $hash
     * @param Cache_Parameters $source
     * @return int
     */
    public function is_xmltv_cache_valid($hash, $source)
    {
        hd_debug_print();

        HD::set_last_error("xmltv_last_error", null);
        $cached_file = $this->get_cache_filename($hash);
        hd_debug_print("Checking cached xmltv file: $cached_file");
        if (!file_exists($cached_file)) {
            hd_debug_print("Cached xmltv file not exist");
            return 1;
        }

        $check_time_file = filemtime($cached_file);
        hd_debug_print("Xmltv cache last modified: " . date("Y-m-d H:i", $check_time_file));

        $expired = true;

        if ((int)$source->ttl === -1) {
            if ($this->curl_wrapper->check_is_expired($source->url)) {
                Curl_Wrapper::clear_cached_etag($source->url);
            } else {
                $expired = false;
            }
        } else if (filesize($cached_file) !== 0) {
            $max_cache_time = 3600 * 24 * (float)$source->ttl;
            if ($check_time_file && $check_time_file + $max_cache_time > time()) {
                $expired = false;
            }
        }

        if ($expired) {
            hd_debug_print("Xmltv cache expired.");
            return 1;
        }

        hd_debug_print("Cached file: $cached_file is not expired");
        $indexed = $this->get_indexes_info($hash);

        if (isset($indexed[self::INDEX_CHANNELS], $indexed[self::INDEX_ENTRIES])
            && $indexed[self::INDEX_CHANNELS] !== -1 && $indexed[self::INDEX_ENTRIES] !== -1) {
            hd_debug_print("All Xmltv indexes are valid");
            return 0;
        }

        if (isset($indexed[self::INDEX_CHANNELS]) && $indexed[self::INDEX_CHANNELS] !== -1) {
            hd_debug_print("Xmltv channels index are valid");
            return 2;
        }

        hd_debug_print("Xmltv cache indexes are invalid");
        return 3;
    }

    /**
     * @param string $hash
     * @param string $ext
     * @return string
     */
    public function get_cache_filename($hash, $ext = ".xmltv")
    {
        return $this->cache_dir . $hash . $ext;
    }

    /**
     * Download XMLTV source.
     *
     * @param string $hash
     * @param Cache_Parameters $source
     * @return int
     */
    public function download_xmltv_source($hash, $source)
    {
        if ($this->is_index_locked($hash)) {
            hd_debug_print("File is indexing or downloading, skipped");
            return 0;
        }

        hd_debug_print_separator();

        $ret = -1;
        $this->perf->reset('start');

        hd_debug_print("Storage space in cache dir: " . HD::get_storage_size($this->cache_dir));
        $cached_file = $this->get_cache_filename($hash);
        $tmp_filename = $cached_file . '.tmp';
        safe_unlink($tmp_filename);

        try {
            HD::set_last_error("xmltv_last_error", null);
            $this->set_index_locked($hash, true);

            if (preg_match("/jtv.?\.zip$/", basename($source->url))) {
                throw new Exception("Unsupported EPG format (JTV)");
            }

            $expired = $this->curl_wrapper->check_is_expired($source->url) || !file_exists($cached_file);
            if (!$expired) {
                hd_debug_print("File not changed, using cached file: $cached_file");
                $this->set_index_locked($hash, false);
                return 1;
            }

            Curl_Wrapper::clear_cached_etag($source->url);
            if (!$this->curl_wrapper->download_file($source->url, $tmp_filename, true)) {
                throw new Exception("Ошибка скачивания $source->url\n\n" . $this->curl_wrapper->get_raw_response_headers());
            }

            if ($this->curl_wrapper->get_http_code() !== 200) {
                throw new Exception("Ошибка скачивания $source->url\n\n" . $this->curl_wrapper->get_raw_response_headers());
            }

            $file_time = filemtime($tmp_filename);
            $dl_time = $this->perf->getReportItemCurrent(Perf_Collector::TIME);
            $file_size = filesize($tmp_filename);
            $bps = $file_size / max($dl_time, 0.001);
            $si_prefix = array('B/s', 'KB/s', 'MB/s');
            $base = 1024;
            $class = min((int)log($bps, $base), count($si_prefix) - 1);
            $class = max($class, 0);
            $speed = sprintf('%1.2f', $bps / pow($base, $class)) . ' ' . $si_prefix[$class];

            hd_debug_print("Last changed time of local file: " . date("Y-m-d H:i", $file_time));
            hd_debug_print("Download $file_size bytes of xmltv source $source->url done in: $dl_time secs (speed $speed)");

            safe_unlink($cached_file);

            $this->perf->setLabel('unpack');

            $handle = fopen($tmp_filename, "rb");
            $hdr = fread($handle, 8);
            fclose($handle);

            if (0 === mb_strpos($hdr, "\x1f\x8b\x08")) {
                hd_debug_print("GZ signature: " . bin2hex(substr($hdr, 0, 3)), true);
                $gz_filename = $cached_file . '.gz';
                if (!rename($tmp_filename, $gz_filename)) {
                    throw new Exception("Failed to rename $tmp_filename to $gz_filename");
                }
                $tmp_filename = $gz_filename;
                hd_debug_print("ungzip $tmp_filename to $cached_file");
                $cmd = "gzip -d " . escapeshellarg($tmp_filename) . " 2>&1";
                $out = system($cmd, $ret);
                if ($ret > 1) {
                    throw new Exception("Failed to unpack $tmp_filename (error code: $ret)\n$out");
                }
                if ($ret === 1 && file_exists($cached_file)) {
                    hd_debug_print("Unpack $tmp_filename with error code: $ret\n$out");
                }
                clearstatcache();
                $size = filesize($cached_file);
                if ($size === 0) {
                    unlink($cached_file);
                    throw new Exception("Unpacked file empty!");
                }
                touch($cached_file, $file_time);
                hd_debug_print("$size bytes ungzipped to $cached_file in "
                    . $this->perf->getReportItemCurrent(Perf_Collector::TIME, 'unpack')
                    . " secs");
            } else if (0 === mb_strpos($hdr, "\x50\x4b\x03\x04")) {
                hd_debug_print("ZIP signature: " . bin2hex(substr($hdr, 0, 4)), true);
                hd_debug_print("unzip $tmp_filename to $cached_file");
                // unpack to separate folder to find out the name of unpacked file
                $unzip_dir = $cached_file . '_unzip';
                self::remove_dir($unzip_dir);
                if (!create_path($unzip_dir)) {
                    throw new Exception("Failed to create folder $unzip_dir");
                }

                $cmd = "unzip -oq " . escapeshellarg($tmp_filename) . " -d " . escapeshellarg($unzip_dir) . " 2>&1";
                $out = system($cmd, $ret);
                safe_unlink($tmp_filename);
                clearstatcache();

                $unpacked = array();
                $files = glob($unzip_dir . '/*');
                if (!empty($files)) {
                    foreach ($files as $file) {
                        if (is_file($file)) {
                            $unpacked[] = $file;
                        }
                    }
                }

                if ($ret !== 0 || count($unpacked) !== 1) {
                    self::remove_dir($unzip_dir);
                    if ($ret !== 0) {
                        throw new Exception("Failed to unpack $tmp_filename (error code: $ret)\n$out");
                    }
                    if (empty($unpacked)) {
                        throw new Exception(TR::t('err_empty_zip__1', $tmp_filename));
                    }
                    throw new Exception("Too many files in zip archive, wrong format??!\n" . implode(PHP_EOL, $unpacked));
                }

                hd_debug_print("unpacked file: $unpacked[0]");
                $moved = rename($unpacked[0], $cached_file);
                self::remove_dir($unzip_dir);
                if (!$moved) {
                    throw new Exception("Failed to rename $unpacked[0] to $cached_file");
                }
                $size = filesize($cached_file);
                touch($cached_file, $file_time);
                hd_debug_print("$size bytes unzipped to $cached_file in " . $this->perf->getReportItemCurrent(Perf_Collector::TIME, 'unpack') . " secs");
            } else if (false !== mb_strpos($hdr, "<?xml")) {
                hd_debug_print("XML signature: " . substr($hdr, 0, 5), true);
                hd_debug_print("rename $tmp_filename to $cached_file");
                rename($tmp_filename, $cached_file);
                $size = filesize($cached_file);
                touch($cached_file, $file_time);
                hd_debug_print("$size bytes stored to $cached_file in " . $this->perf->getReportItemCurrent(Perf_Collector::TIME, 'unpack') . " secs");
            } else {
                hd_debug_print("Unknown signature: " . bin2hex($hdr), true);
                throw new Exception(TR::load('err_unknown_file_type'));
            }

            $ret = 1;
            $this->set_index_locked($hash, false);
            $this->remove_all_indexes($hash);
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            safe_unlink($tmp_filename);
            safe_unlink($cached_file);
            $this->set_index_locked($hash, false);
        }

        hd_debug_print_separator();

        return $ret;
    }

    /**
     * @param string $hash
     * @return bool
     */
    public function is_index_locked($hash)
    {
        $dirs = glob($this->cache_dir . $hash . '_*.lock', GLOB_ONLYDIR);
        return !empty($dirs);
    }

    /**
     * @return bool|array
     */
    public function is_any_index_locked()
    {
        $locks = array();
        $dirs = array();
        if ($this->active_sources->size() === 0) {
            $dirs = glob($this->cache_dir . '*_*.lock', GLOB_ONLYDIR);
        } else {
            foreach ($this->active_sources as $key => $value) {
                $dirs = safe_merge_array($dirs, glob($this->cache_dir . $key . '_*.lock', GLOB_ONLYDIR));
            }
        }

        foreach ($dirs as $dir) {
            $locks[] = basename($dir);
        }
        return empty($locks) ? false : $locks;
    }

    /**
     * @param string $hash
     * @param bool $lock
     */
    public function set_index_locked($hash, $lock)
    {
        $lock_dir = $this->get_cache_filename($hash, "_$this->pid.lock");
        if ($lock) {
            if (!create_path($lock_dir, 0644)) {
                hd_debug_print("Directory '$lock_dir' was not created");
            } else {
                hd_debug_print("Lock $lock_dir");
            }
        } else if (is_dir($lock_dir)) {
            hd_debug_print("Unlock $lock_dir");
            self::remove_dir($lock_dir);
        }
    }

    /**
     * clear memory cache and cache for selected filename (hash) mask
     *
     * @return void
     */
    public function clear_all_epg_files()
    {
        hd_debug_print(null, true);
        Curl_Wrapper::save_cached_etags(array());
        $this->clear_memory_index();

        if (empty($this->cache_dir)) {
            return;
        }

        $dirs = glob($this->cache_dir . "*_*.lock", GLOB_ONLYDIR);
        $locks = array();
        foreach ($dirs as $dir) {
            hd_debug_print("Found locks: $dir");
            $locks[] = $dir;
        }

        if (!empty($locks)) {
            foreach ($locks as $lock) {
                $ar = explode('_', basename($lock));
                $pid = (int)end($ar);

                if ($pid !== 0 && send_process_signal($pid, 0)) {
                    hd_debug_print("Kill process $pid");
                    send_process_signal($pid, 9);
                    // give a time to terminate process
                    sleep(1);
                }
                hd_debug_print("Remove lock: $lock");
                self::remove_dir($lock);
            }
        }

        // cache dir can be selected by user, remove only files created by plugin
        hd_debug_print("clear epg files in: $this->cache_dir");
        $masks = array('*.xmltv', '*.xmltv.tmp', '*.xmltv.gz', '*.index', '*.db', '*.db-journal', '*_indexing.log', '*.cache', 'curl_*');
        foreach ($masks as $mask) {
            $files = glob($this->cache_dir . $mask);
            if (empty($files)) continue;

            foreach ($files as $file) {
                safe_unlink($file);
            }
        }

        $dirs = glob($this->cache_dir . '*.xmltv_unzip', GLOB_ONLYDIR);
        if (!empty($dirs)) {
            foreach ($dirs as $dir) {
                self::remove_dir($dir);
            }
        }
        clearstatcache();
        hd_debug_print("Storage space in cache dir: " . HD::get_storage_size($this->cache_dir));
    }

    public function clear_stalled_locks()
    {
        $locks = $this->is_any_index_locked();
        if ($locks !== false) {
            foreach ($locks as $lock) {
                $ar = explode('_', $lock);
                $pid = (int)end($ar);

                if ($pid !== 0 && !send_process_signal($pid, 0)) {
                    hd_debug_print("Remove stalled lock: $lock");
                    self::remove_dir($this->cache_dir . $lock);
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// abstract methods

    /**
     * Remove is selected index
     *
     * @param string $name
     * @param string $hash
     * @return bool
     */
    abstract public function remove_index($name, $hash);

    /**
     * Remove is selected index
     *
     * @param string $hash
     */
    abstract public function remove_all_indexes($hash);

    /**
     * Get information about indexes
     * @param string $hash
     * @return array
     */
    abstract public function get_indexes_info($hash);

    /**
     * @param string $hash
     * @param Channel $channel
     * @return array
     */
    abstract public function get_epg_id($hash, $channel);

    /**
     * Clear memory index
     *
     * @param string $id
     * @return void
     */
    abstract protected function clear_memory_index($id = '');

    /**
     * @param string $hash
     * @param Channel $channel
     * @return array
     */
    abstract public function load_program_index($hash, $channel);

    /**
     * Check is all indexes is valid
     *
     * @param array $names
     * @param string $hash
     * @return bool
     */
    abstract protected function is_all_indexes_valid($names, $hash);

    ///////////////////////////////////////////////////////////////////////////////
    /// protected methods

    /**
     * Remove directory with content
     *
     * @param string $dir
     * @return void
     */
    protected static function remove_dir($dir)
    {
        if (!empty($dir) && is_dir($dir)) {
            shell_exec('rm -rf ' . escapeshellarg($dir));
            clearstatcache();
        }
    }

    /**
     * Collect all <channel> elements of xmltv file and pass them to $store in batches.
     *
     * The elements are cut out of a forward-only buffer and parsed in batches by single
     * DOMDocument. XMLTV declares <!ELEMENT tv (channel*, programme*)> so scanning stops
     * when programmes starts instead of reading hundreds of Mb of the <programme> data.
     *
     * $store receives array of channels, every channel is
     * array('id' => channel id, 'picon' => picon url or '', 'aliases' => array of display-name)
     *
     * @param resource $file
     * @param callable $store
     * @return int number of channels found
     */
    protected static function scan_xmltv_channels($file, $store)
    {
        $use_errors = libxml_use_internal_errors(true);

        $total = 0;
        $buffer = '';
        $pending = array();
        $eof = false;
        $done = false;
        while (!$done) {
            if (!$eof) {
                $chunk = fread($file, self::CHANNELS_BLOCK_SIZE);
                if ($chunk === false || $chunk === '') {
                    $eof = true;
                } else {
                    $buffer .= $chunk;
                    $eof = feof($file);
                }
            }

            // cut out every <channel ...> ... </channel> the buffer already holds
            $consumed = 0;
            while (($start = strpos($buffer, '<channel ', $consumed)) !== false) {
                $end = strpos($buffer, '</channel>', $start + 9);
                if ($end === false) break;
                $pending[] = substr($buffer, $start, $end + 10 - $start);
                $consumed = $end + 10;
            }

            if ($start === false) {
                // every <channel> is placed before the first <programme>. Bail out only after at least
                // one channel was seen, so a source using an unusual order is still handled.
                if (($total !== 0 || !empty($pending)) && strpos($buffer, '<programme', $consumed) !== false) {
                    $done = true;
                }
                // keep a short tail so an open tag split across two reads is still matched
                $keep = strlen($buffer) - 9;
                $buffer = ($keep > $consumed) ? substr($buffer, $keep) : substr($buffer, $consumed);
            } else if (strlen($buffer) - $start > self::MAX_CHANNEL_ELEMENT_SIZE) {
                // unterminated <channel> - drop it instead of buffering the rest of the file
                hd_debug_print("Unterminated <channel> element, skipped");
                $buffer = substr($buffer, $start + 9);
            } else {
                $buffer = substr($buffer, $start);
            }

            if ($eof) {
                $done = true;
            }

            if (!empty($pending) && ($done || count($pending) >= self::CHANNELS_BATCH_SIZE)) {
                $channels = self::parse_channels_batch($pending);
                $pending = array();
                if (!empty($channels)) {
                    $total += count($channels);
                    call_user_func($store, $channels);
                }
            }
        }

        libxml_clear_errors();
        libxml_use_internal_errors($use_errors);

        return $total;
    }

    /**
     * Parse a batch of raw <channel> elements with one DOMDocument
     *
     * @param array $fragments raw '<channel ...>...</channel>' strings
     * @return array
     */
    protected static function parse_channels_batch($fragments)
    {
        $xml_node = new DOMDocument();
        if (!$xml_node->loadXML('<tv>' . implode('', $fragments) . '</tv>', LIBXML_NOWARNING | LIBXML_NOERROR)) {
            libxml_clear_errors();
            if (count($fragments) === 1) {
                hd_debug_print("Malformed <channel> element skipped: " . substr($fragments[0], 0, 256), true);
                return array();
            }

            // a single malformed element must not cost the whole batch
            $channels = array();
            foreach ($fragments as $fragment) {
                $channels = array_merge($channels, self::parse_channels_batch(array($fragment)));
            }
            return $channels;
        }

        $channels = array();
        foreach ($xml_node->getElementsByTagName('channel') as $tag) {
            $channel_id = $tag->getAttribute('id');
            if (empty($channel_id)) continue;

            $picon = '';
            foreach ($tag->getElementsByTagName('icon') as $icon) {
                $src = $icon->getAttribute('src');
                if (!empty($src) && preg_match(HTTP_PATTERN, $src)) {
                    $picon = $src;
                    break;
                }
            }

            $aliases = array();
            foreach ($tag->getElementsByTagName('display-name') as $name) {
                $aliases[] = $name->nodeValue;
            }

            $channels[] = array('id' => $channel_id, 'picon' => $picon, 'aliases' => $aliases);
        }

        return $channels;
    }

    /**
     * Find blocks of the <programme> elements that belongs to the same channel
     * and pass it to $store($channel_id, $start, $end). $start - position of the first open tag of the block,
     * $end - position of the open tag of the next block or position of the '</tv>' for the last block.
     *
     * The file is walked in INDEX_BLOCK_SIZE blocks and the <programme> open tags are read straight
     * out of the block instead of reading every programme element (stream_get_line() per programme
     * allocates whole element body just to look at the channel attribute in its open tag).
     *
     * A row is only produced where the channel changes, so the scan has two strategies:
     *
     *  - $find_other is a regex for "a channel attribute that is not the current channel".
     *    One call skips every programme of the current channel in one sweep inside pcre
     *    instead of one php call per open tag. Worth it only while channel changes are rare,
     *    because each change compiles a new pattern.
     *
     *  - otherwise the open tags are walked one at a time. Inside a source the channel
     *    attribute sits at the same offset in every open tag (the start/stop timestamps in
     *    front of it have a fixed width), so $probe holds 'channel="<current id>"' and
     *    $attr_offset where it was last seen, and the walk costs one compare against the
     *    block rather than a search for the attribute plus a comparison of its value.
     *
     * Which one runs is decided per block from what the previous block looked like.
     *
     * @param resource $file
     * @param callable $store
     * @return int number of stored blocks
     */
    protected static function scan_xmltv_positions($file, $store)
    {
        $stat = fstat($file);
        $file_size = $stat['size'];

        $stored = 0;
        $prev_channel = null;
        $start_program_block = 0;
        $block_pos = 0;
        $attr_offset = 0;
        $probe = null;
        $probe_len = 0;
        $probe_end = 0;
        $use_regex = false;
        $since_sample = 0;
        $end_found = false;

        while ($block_pos < $file_size) {
            fseek($file, $block_pos);
            $block = fread($file, self::INDEX_BLOCK_SIZE);
            if ($block === false || $block === '') break;

            $block_len = strlen($block);
            $last_block = ($block_pos + $block_len >= $file_size);

            // Positions at or after $limit are left to the next block: the open tag starting
            // there may be cut in half by the block boundary.
            $limit = $block_len;
            $next_block_pos = $block_pos + $block_len;
            if (!$last_block) {
                $last_open = strrpos($block, '<programme');
                if ($last_open === false) {
                    // nothing here, keep 9 bytes in case '<programme' straddles the block edge
                    $block_pos += $block_len - 9;
                    continue;
                }

                if ($last_open === 0) {
                    // a single element longer than the block - its open tag is at the very
                    // start so it is complete, the rest of the block is element body
                    $next_block_pos = $block_pos + $block_len - 9;
                } else {
                    $limit = $last_open;
                    $next_block_pos = $block_pos + $last_open;
                }
            }

            $find_other = (!$use_regex || $prev_channel === null || $since_sample >= self::RESAMPLE_BLOCKS)
                ? null
                : '~channel="(?!' . preg_quote($prev_channel, '~') . '")~';

            $changes = 0;
            $tags = 0;
            $pos = 0;
            while ($pos < $limit) {
                if ($find_other === null) {
                    // walk the open tags one at a time
                    $pos = strpos($block, '<programme', $pos);
                    if ($pos === false || $pos >= $limit) break;

                    $tags++;
                    // same channel, same layout as the tag before it - nothing to do here
                    if ($probe !== null && $pos + $probe_end <= $block_len
                        && substr_compare($block, $probe, $pos + $attr_offset, $probe_len) === 0) {
                        $pos += 10;
                        continue;
                    }

                    $ch_start = strpos($block, 'channel="', $pos);
                    if ($ch_start === false) break;

                    // guard against picking up the next element's attribute when this open tag
                    // carries no channel= at all (cheap distance check first, '>' scan only if odd)
                    if ($ch_start - $pos > 512) {
                        $tag_end = strpos($block, '>', $pos);
                        if ($tag_end !== false && $ch_start > $tag_end) {
                            $pos += 10;
                            continue;
                        }
                    }

                    $tag_start = $pos;
                    $new_offset = $ch_start - $pos;
                } else {
                    // jump over every programme that still belongs to the current channel
                    if (!preg_match($find_other, $block, $match, PREG_OFFSET_CAPTURE, $pos)) break;

                    $ch_start = $match[0][1];
                    if ($ch_start + 9 >= $limit) break;

                    // the attribute has to belong to an open tag and not to element content
                    $tag_start = strrpos($block, '<programme', $ch_start - $block_len);
                    if ($tag_start === false) {
                        $pos = $ch_start + 9;
                        continue;
                    }
                    $tag_end = strpos($block, '>', $tag_start);
                    if ($tag_end !== false && $tag_end < $ch_start) {
                        $pos = $ch_start + 9;
                        continue;
                    }

                    $new_offset = $ch_start - $tag_start;
                }
                $ch_start += 9;

                $ch_end = strpos($block, '"', $ch_start);
                if ($ch_end === false) break;

                $channel_id = substr($block, $ch_start, $ch_end - $ch_start);
                if ($channel_id === '' || $channel_id === false) {
                    $pos = $ch_start;
                    continue;
                }

                $attr_offset = $new_offset;
                $probe = 'channel="' . $channel_id . '"';
                $probe_len = strlen($probe);
                $probe_end = $attr_offset + $probe_len;

                if ($channel_id !== $prev_channel) {
                    $tag_start_pos = $block_pos + $tag_start;
                    if ($prev_channel !== null) {
                        // close the previous channel block at this open tag
                        call_user_func($store, self::decode_attribute($prev_channel), $start_program_block, $tag_start_pos);
                        $stored++;
                    }

                    $prev_channel = $channel_id;
                    $start_program_block = $tag_start_pos;
                    $changes++;
                    if ($find_other !== null) {
                        $find_other = '~channel="(?!' . preg_quote($channel_id, '~') . '")~';
                    }
                }
                $pos = $ch_start;
            }

            // pick the strategy for the next block from what this one looked like
            if ($find_other === null) {
                $since_sample = 0;
                $use_regex = ($tags >= self::SPARSE_BLOCK_TAGS && $changes <= self::DENSE_BLOCK_TRANSITIONS);
            } else {
                $since_sample++;
                if ($changes > self::DENSE_BLOCK_TRANSITIONS) {
                    $use_regex = false;
                }
            }

            if ($last_block) {
                // close the trailing block at </tv>
                $end_tv = strpos($block, '</tv>');
                if ($end_tv !== false && $prev_channel !== null) {
                    call_user_func($store, self::decode_attribute($prev_channel), $start_program_block, $block_pos + $end_tv);
                    $stored++;
                    $end_found = true;
                }
                break;
            }

            $block_pos = $next_block_pos;
        }

        if (!$end_found && $prev_channel !== null) {
            // truncated file without </tv>, close the trailing block at the end of file
            hd_debug_print("Closing tag </tv> not found, file may be truncated");
            call_user_func($store, self::decode_attribute($prev_channel), $start_program_block, $file_size);
            $stored++;
        }

        return $stored;
    }

    /**
     * Decode xml entities in raw attribute value read from the file.
     * Channel id in the channels index is decoded by DOM, positions must use the same value.
     *
     * @param string $value
     * @return string
     */
    protected static function decode_attribute($value)
    {
        if (strpos($value, '&') === false) {
            return $value;
        }

        // &apos; is not known by html_entity_decode in php 5.3
        return html_entity_decode(str_replace('&apos;', "'", $value), ENT_QUOTES, 'UTF-8');
    }

    /**
     * @param string $hash
     * @return resource
     * @throws Exception
     */
    protected function open_xmltv_file($hash)
    {
        $cached_file = $this->get_cache_filename($hash);
        if (!file_exists($cached_file)) {
            throw new Exception("cache file $cached_file not exist");
        }

        $file = fopen($cached_file, 'rb');
        if (!$file) {
            throw new Exception("can't open $cached_file");
        }

        return $file;
    }
}

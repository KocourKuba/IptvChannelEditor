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
        $this->cache_dir = $cache_dir;
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

        hd_debug_print("Processing source: " . pretty_json_format($source), true);

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
            $this->curl_wrapper->set_url($source->url);
            if ($this->curl_wrapper->check_is_expired()) {
                $this->curl_wrapper->clear_cached_etag();
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
        return $this->cache_dir . DIRECTORY_SEPARATOR . $hash . $ext;
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
        if (file_exists($tmp_filename)) {
            unlink($tmp_filename);
        }

        try {
            HD::set_last_error("xmltv_last_error", null);
            $this->set_index_locked($hash, true);

            if (preg_match("/jtv.?\.zip$/", basename($source->url))) {
                throw new Exception("Unsupported EPG format (JTV)");
            }

            $this->curl_wrapper->set_url($source->url);
            $expired = $this->curl_wrapper->check_is_expired() || !file_exists($tmp_filename);
            if (!$expired) {
                hd_debug_print("File not changed, using cached file: $cached_file");
                $this->set_index_locked($hash, false);
                return 1;
            }

            $this->curl_wrapper->clear_cached_etag();
            if (!$this->curl_wrapper->download_file($tmp_filename, true)) {
                throw new Exception("Ошибка скачивания $source->url\n\n" . $this->curl_wrapper->get_raw_response_headers());
            }

            if ($this->curl_wrapper->get_response_code() !== 200) {
                throw new Exception("Ошибка скачивания $source->url\n\n" . $this->curl_wrapper->get_raw_response_headers());
            }

            $file_time = filemtime($tmp_filename);
            $dl_time = $this->perf->getReportItemCurrent(Perf_Collector::TIME);
            $file_size = filesize($tmp_filename);
            $bps = $file_size / $dl_time;
            $si_prefix = array('B/s', 'KB/s', 'MB/s');
            $base = 1024;
            $class = min((int)log($bps, $base), count($si_prefix) - 1);
            $speed = sprintf('%1.2f', $bps / pow($base, $class)) . ' ' . $si_prefix[$class];

            hd_debug_print("Last changed time of local file: " . date("Y-m-d H:i", $file_time));
            hd_debug_print("Download $file_size bytes of xmltv source $source->url done in: $dl_time secs (speed $speed)");

            if (file_exists($cached_file)) {
                unlink($cached_file);
            }

            $this->perf->setLabel('unpack');

            $handle = fopen($tmp_filename, "rb");
            $hdr = fread($handle, 8);
            fclose($handle);

            if (0 === mb_strpos($hdr, "\x1f\x8b\x08")) {
                hd_debug_print("GZ signature: " . bin2hex(substr($hdr, 0, 3)), true);
                rename($tmp_filename, $cached_file . '.gz');
                $tmp_filename = $cached_file . '.gz';
                hd_debug_print("ungzip $tmp_filename to $cached_file");
                $cmd = "gzip -d $tmp_filename 2>&1";
                system($cmd, $ret);
                if ($ret !== 0) {
                    throw new Exception("Failed to unpack $tmp_filename (error code: $ret)");
                }
                clearstatcache();
                $size = filesize($cached_file);
                touch($cached_file, $file_time);
                hd_debug_print("$size bytes ungzipped to $cached_file in " . $this->perf->getReportItemCurrent(Perf_Collector::TIME, 'unpack') . " secs");
            } else if (0 === mb_strpos($hdr, "\x50\x4b\x03\x04")) {
                hd_debug_print("ZIP signature: " . bin2hex(substr($hdr, 0, 4)), true);
                hd_debug_print("unzip $tmp_filename to $cached_file");
                $filename = trim(shell_exec("unzip -lq '$tmp_filename'|grep -E '[\d:]+'"));
                if (empty($filename)) {
                    throw new Exception(TR::t('err_empty_zip__1', $tmp_filename));
                }

                if (explode('\n', $filename) > 1) {
                    throw new Exception("Too many files in zip archive, wrong format??!\n$filename");
                }

                hd_debug_print("zip list: $filename");
                $cmd = "unzip -oq $tmp_filename -d $this->cache_dir 2>&1";
                system($cmd, $ret);
                unlink($tmp_filename);
                if ($ret !== 0) {
                    throw new Exception("Failed to unpack $tmp_filename (error code: $ret)");
                }
                clearstatcache();

                rename($filename, $cached_file);
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
                throw new Exception(TR::load_string('err_unknown_file_type'));
            }

            $ret = 1;
            $this->set_index_locked($hash, false);
            $this->remove_all_indexes($hash);
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            if (!empty($tmp_filename) && file_exists($tmp_filename)) {
                unlink($tmp_filename);
            }

            if (file_exists($cached_file)) {
                unlink($cached_file);
            }
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
        $dirs = glob($this->cache_dir . DIRECTORY_SEPARATOR . $hash . "_*.lock", GLOB_ONLYDIR);
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
            $dirs = glob($this->cache_dir . DIRECTORY_SEPARATOR . "*_*.lock", GLOB_ONLYDIR);
        } else {
            foreach ($this->active_sources as $key => $value) {
                $dirs = safe_merge_array($dirs, glob($this->cache_dir . DIRECTORY_SEPARATOR . $key . "_*.lock", GLOB_ONLYDIR));
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
            shell_exec("rm -rf $lock_dir");
            clearstatcache();
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
        $this->curl_wrapper->clear_all_etag_cache();
        $this->clear_memory_index();

        if (empty($this->cache_dir)) {
            return;
        }

        $dirs = glob($this->cache_dir . DIRECTORY_SEPARATOR . "*_*.lock", GLOB_ONLYDIR);
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
                    send_process_signal($pid, -9);
                }
                hd_debug_print("Remove lock: $lock");
                shell_exec("rm -rf $lock");
            }
        }

        $files = $this->cache_dir . DIRECTORY_SEPARATOR . "*";
        hd_debug_print("clear epg files: $files");
        shell_exec('rm -rf ' . $files);
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
                    shell_exec("rmdir $this->cache_dir" . DIRECTORY_SEPARATOR . $lock);
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
    abstract protected function load_program_index($hash, $channel);

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

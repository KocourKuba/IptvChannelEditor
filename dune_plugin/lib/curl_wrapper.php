<?php
/**
 * The MIT License (MIT)
 *
 * @Author: sharky72 (https://github.com/KocourKuba)
 * Some code imported from various authors of dune plugins
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

class Curl_Wrapper
{
    const CACHE_TAG_FILE = "etag_cache.dat";

    /**
     * @var int
     */
    private $connect_timeout = 30;

    /**
     * @var int
     */
    private $download_timeout = 60;

    /**
     * @var array
     */
    private $send_headers;

    /**
     * @var array
     */
    private $options;

    /**
     * @var array
     */
    private $post_data;

    /**
     * @var bool
     */
    private $is_post = false;

    /**
     * @var int
     */
    private static $http_code;

    /**
     * @var int
     */
    private static $error_no;

    /**
     * @var string
     */
    private static $error_desc;

    /**
     * @var array|null
     */
    private static $http_response_headers = null;

    public function __construct()
    {
        create_path(get_data_path());
        $this->reset();
    }

    public function reset()
    {
        hd_debug_print(null, true);
        self::$http_response_headers = null;
        self::$error_no = 0;
        self::$error_desc = '';
        self::$http_code = 0;
        $this->send_headers = array();
        $this->is_post = false;
        $this->post_data = null;
    }

    public static function getInstance()
    {
        return new self();
    }

    /**
     * download file to selected path
     *
     * @param string $url
     * @param string $save_file path to file
     * @param bool $use_cache use ETag caching
     * @return bool result of operation
     */
    public function download_file($url, $save_file, $use_cache = false)
    {
        hd_debug_print(null, true);

        return $this->exec_curl($url, $save_file, $use_cache);
    }

    /**
     * download and return contents
     *
     * @param string $url
     * @param bool $use_cache use ETag caching
     * @return string|bool content of the downloaded file or result of operation
     */
    public function download_content($url, $use_cache = false)
    {
        hd_debug_print(null, true);

        return $this->exec_curl($url, null, $use_cache);
    }

    /**
     * @return string
     */
    public static function get_etag_header()
    {
        return self::get_response_header('etag');
    }

    /**
     * @return string
     */
    public static function get_response_header($header)
    {
        return safe_get_value(self::get_response_headers(), $header, '');
    }

    /**
     * @param $value
     */
    public function set_post($value = true)
    {
        $this->is_post = $value;
    }

    /**
     * @param array $headers
     */
    public function set_send_headers($headers)
    {
        $this->send_headers = $headers;
    }

    /**
     * @param array $opts
     */
    public function set_options($opts)
    {
        $this->options = $opts;
    }

    /**
     * @param array $data
     */
    public function set_post_data($data)
    {
        $this->post_data = $data;
    }

    /**
     * @return array
     */
    public static function get_response_headers()
    {
        return empty(self::$http_response_headers) ? array() : self::$http_response_headers;
    }

    /**
     * @return string
     */
    public function get_raw_response_headers()
    {
        $headers = array();
        foreach ($this->get_response_headers() as $key => $header) {
            $headers[] = "$key: $header";
        }
        return implode(PHP_EOL, $headers);
    }

    /**
     * @param  int $timeout
     */
    public function set_connect_timeout($timeout)
    {
        $this->connect_timeout = $timeout;
    }

    /**
     * @param int $timeout
     */
    public function set_download_timeout($timeout)
    {
        $this->download_timeout = $timeout;
    }

    /**
     * Check if cached url is expired
     *
     * @param string $url
     * @return bool result of operation
     */
    public function check_is_expired($url)
    {
        hd_debug_print(null, true);

        $etag = self::get_cached_etag($url);
        if (empty($etag)) {
            hd_debug_print("No ETag value");
        } else {
            if ($this->exec_curl($url, false, true)) {
                $code = $this->get_http_code();
                hd_debug_print("http code: $code", true);
                return !($code === 304 || ($code === 200 && $this->get_etag_header() === $etag));
            }
        }

        return true;
    }

    /**
     * @return int
     */
    public function get_http_code()
    {
        return self::$http_code;
    }

    /**
     * @return int
     */
    public function get_error_no()
    {
        return self::$error_no;
    }

    /**
     * @return string
     */
    public function get_error_desc()
    {
        return self::$error_desc;
    }


    /////////////////////////////////////////////////////////////
    /// static functions

    /**
     * @param string $url
     */
    public static function get_url_hash($url)
    {
        return hash('crc32', $url);
    }

    /**
     * @param bool $is_file
     * @param string $source contains data or file name
     * @param bool $assoc
     * @return mixed|false
     */
    public static function decodeJsonResponse($is_file, $source, $assoc = false)
    {
        if ($source === false) {
            return false;
        }

        if ($is_file) {
            $data = file_get_contents($source);
        } else {
            $data = $source;
        }

        $contents = json_decode($data, $assoc);
        if ($contents !== null && $contents !== false) {
            return $contents;
        }

        hd_debug_print("failed to decode json");
        hd_debug_print("doc: $data", true);

        return false;
    }

    /**
     * @param string $url
     * @return string
     */
    public static function get_cached_etag($url)
    {
        if (empty($url)) {
            return '';
        }

        $cache_db = self::load_cached_etags();
        $hash = self::get_url_hash($url);
        return safe_get_value($cache_db, $hash, '');
    }

    /**
     * @param string $url
     * @param string $etag
     * @return void
     */
    public static function set_cached_etag($url, $etag)
    {
        if (!empty($url) && !empty($etag)) {
            $cache_db = self::load_cached_etags();
            $hash = self::get_url_hash($url);
            $cache_db[$hash] = $etag;
            self::save_cached_etags($cache_db);
        }
    }

    /**
     * @param string $url
     * @return void
     */
    public static function clear_cached_etag($url)
    {
        if (!empty($url)) {
            $cache_db = self::load_cached_etags();
            $hash = self::get_url_hash($url);
            unset($cache_db[$hash]);
            self::save_cached_etags($cache_db);
        }
    }

    /**
     * @return array
     */
    protected static function load_cached_etags()
    {
        $etag_cache_file = get_data_path(self::CACHE_TAG_FILE);
        if (file_exists($etag_cache_file)) {
            $cache_db = json_decode(file_get_contents($etag_cache_file), true);
        }

        if (!isset($cache_db) ||$cache_db === false) {
            $cache_db = array();
        }

        return $cache_db;
    }

    /**
     * @param array $cache_db
     * @return void
     */
    public static function save_cached_etags($cache_db)
    {
        file_put_contents(get_data_path(self::CACHE_TAG_FILE), json_encode($cache_db));
    }

    /** @noinspection PhpUnusedParameterInspection */
    public static function http_header_function($curl, $header)
    {
        $len = strlen($header);
        $header = explode(':', $header, 2);
        if (count($header) == 2) {
            $key = strtolower(trim($header[0]));
            self::$http_response_headers[$key] = trim($header[1]);
        }
        return $len;
    }

    /////////////////////////////////////////////////////////////
    /// private functions


    /**
     * if $save_file == null return content of request
     * if $save_file == false return only result of request
     *
     * @param string $url
     * @param string|null|bool $save_file
     * @param bool $use_cache
     * @return bool|string
     */
    protected function exec_curl($url, $save_file, $use_cache = false)
    {
        if (is_r20_or_higher()) {
            $res = $this->exec_php_curl($url, $save_file, $use_cache);
        } else {
            $res = $this->exec_shell_curl($url, $save_file, $use_cache);
        }

        return $res;
    }

    /**
     * if $save_file == null return content of request
     * if $save_file == false return only result of request
     *
     * @param string $url
     * @param string|null|bool $save_file
     * @param bool $use_cache
     * @return bool|string
     */
    protected function exec_shell_curl($url, $save_file, $use_cache = false)
    {
        hd_debug_print("curl: '$url' saved to '$save_file' use cache: " . var_export($use_cache, true), true);
        self::$http_code = 0;
        self::$error_no = 0;

        $url_hash = self::get_url_hash($url);
        $logfile = get_temp_path("{$url_hash}_response.log");
        safe_unlink($logfile);
        $headers_path = get_temp_path("{$url_hash}_headers.log");
        $temp_file = tempnam(get_temp_path(), 'dl_');

        self::$http_response_headers = null;

        $config_data[] = "--insecure";
        $config_data[] = "--silent";
        $config_data[] = "--fail";
        $config_data[] = "--show-error";
        $config_data[] = "--dump-header $headers_path";
        $config_data[] = "--connect-timeout $this->connect_timeout";
        $config_data[] = "--max-time $this->download_timeout";
        $config_data[] = "--location";
        $config_data[] = "--max-redirs 5";
        $config_data[] = "--compressed";
        $config_data[] = "--parallel";
        $config_data[] = "--write-out \"RESPONSE_CODE: %{response_code}\"";
        $config_data[] = "--user-agent \"" . HD::get_dune_user_agent() . "\"";
        $config_data[] = "--url \"$url\"";

        foreach ($this->send_headers as $header) {
            $config_data[] = "--header \"$header\"";
        }

        if (isset($this->options[CURLOPT_INFILE]) || isset($this->options[CURLOPT_INFILESIZE])) {
            $config_data[] = "--request PUT";
        } else if ($save_file === false) {
            $config_data[] = "--head";
            $config_data[] = "--output /dev/null";
        } else if ($save_file !== null){
            hd_debug_print("Save to file: '$save_file'", true);
            $config_data[] = "--output \"$save_file\"";
        } else {
            hd_debug_print("Save to temp file: '$temp_file'", true);
            $config_data[] = "--output \"$temp_file\"";
        }

        if ($this->is_post) {
            $config_data[] = "--request POST";
        }

        if ($use_cache) {
            $etag = self::get_cached_etag($url);
            if (!empty($etag)) {
                $header = "If-None-Match: " . str_replace('"', '\"', $etag);
                $config_data[] = "--header \"$header\"";
            }
        }

        if (!empty($this->post_data)) {
            $data = '';
            if (isset($this->send_headers) && in_array(CONTENT_TYPE_JSON, $this->send_headers)) {
                $data = escaped_raw_json_encode($this->post_data);
            } else {
                foreach($this->post_data as $key => $value) {
                    if (!empty($data)) {
                        $data .= "&";
                    }
                    $data .= $key . "=" . urlencode($value);
                }
            }

            $config_data[] = "--data \"$data\"";
        }

        if (LogSeverity::$is_debug) {
            hd_debug_print("Curl config:");
            foreach ($config_data as $line) {
                hd_debug_print($line);
            }
        }

        $config_file = get_temp_path("{$url_hash}_curl_config.txt");
        file_put_contents($config_file, implode(PHP_EOL, $config_data));

        $cmd = get_platform_curl() . " --config $config_file >>$logfile";
        hd_debug_print("Exec: $cmd", true);
        $output = '';
        $result = exec($cmd, $output, self::$error_no);
        if ($result === false) {
            hd_debug_print("Problem with exec curl");
            return false;
        }

        if (!file_exists($logfile)) {
            $log_content = "No http_proxy log! Exec result code: $result";
            hd_debug_print($log_content);
        } else {
            $log_content = file_get_contents($logfile);
            $pos = strpos($log_content, "RESPONSE_CODE:");
            if ($pos !== false) {
                self::$http_code = (int)trim(substr($log_content, $pos + strlen("RESPONSE_CODE:")));
                hd_debug_print("Response code: " . self::$http_code, true);
            }
            safe_unlink($logfile);
        }

        if (file_exists($headers_path)) {
            $headers = file_get_contents($headers_path);
            if (!empty($headers)) {
                $lines = explode("\r\n", $headers);
                foreach ($lines as $line) {
                    if (!empty($line)) {
                        self::http_header_function(null, $line);
                    }
                }

                if (!empty(self::$http_response_headers) && LogSeverity::$is_debug) {
                    hd_debug_print("---------  Response headers start ---------");
                    foreach (self::$http_response_headers as $key => $header) {
                        hd_debug_print("$key: $header");
                    }
                    hd_debug_print("---------   Response headers end  ---------");
                }
            }

            if ($use_cache) {
                $new_etag = self::get_etag_header();
                if (!empty($new_etag) && $etag !== $new_etag) {
                    hd_debug_print("Save new ETag ($new_etag) for: $url", true);
                    self::set_cached_etag($url, $new_etag);
                }
            }
        }

        if ($save_file === null) {
            if (file_exists($temp_file)) {
                $content = file_get_contents($temp_file);
                safe_unlink($temp_file);
                return $content;
            }
            return false;
        }

        return self::$error_no === 0;
    }

    /**
     * if $save_file == null return content of request
     * if $save_file == false return only result of request
     *
     * @param string $url
     * @param string|null|bool $save_file
     * @param bool $use_cache
     * @return bool|string
     */
    private function exec_php_curl($url, $save_file, $use_cache = false)
    {
        hd_debug_print("exec_php_curl: url: '$url'", true);
        if ($save_file === false) {
            hd_debug_print("exec_php_curl: request only headers", true);
        }

        self::$http_code = 0;
        self::$http_response_headers = null;

        $opts[CURLOPT_URL] = $url;
        $opts[CURLOPT_SSL_VERIFYPEER] = 0;
        $opts[CURLOPT_SSL_VERIFYHOST] = 0;
        $opts[CURLOPT_CONNECTTIMEOUT] = $this->connect_timeout;
        $opts[CURLOPT_TIMEOUT] = $this->download_timeout;
        $opts[CURLOPT_RETURNTRANSFER] = 1;
        $opts[CURLOPT_FOLLOWLOCATION] = 1;
        $opts[CURLOPT_MAXREDIRS] = 5;
        $opts[CURLOPT_FILETIME] = 1;
        $opts[CURLOPT_USERAGENT] = HD::get_dune_user_agent();
        $opts[CURLOPT_HEADERFUNCTION] = 'Curl_Wrapper::http_header_function';
        $opts[CURLOPT_ENCODING] = "";

        if (!empty($this->options)) {
            $opts = safe_merge_array($opts, $this->options);
        }

        $fp = null;
        if (isset($opts[CURLOPT_INFILE]) || isset($opts[CURLOPT_INFILESIZE])) {
            $opts[CURLOPT_PUT] = 1;
        } else if ($save_file === false) {
            $opts[CURLOPT_NOBODY] = 1;
        } else if ($save_file !== null){
            hd_debug_print("Save to file: '$save_file'", true);
            $fp = fopen($save_file, "w+");
            $opts[CURLOPT_FILE] = $fp;
        }

        if (!empty($this->post_data)) {
            if (isset($this->send_headers) && in_array(CONTENT_TYPE_JSON, $this->send_headers)) {
                $opts[CURLOPT_POSTFIELDS] = json_format_unescaped($this->post_data);
            } else {
                $opts[CURLOPT_POSTFIELDS] = http_build_query($this->post_data);
            }
            $opts[CURLOPT_HTTPHEADER][] = "Content-Length: " . strlen($opts[CURLOPT_POSTFIELDS]);
        }

        if ($this->is_post) {
            $opts[CURLOPT_POST] = $this->is_post;
        } else if (empty($opts[CURLOPT_NOBODY]) && empty($opts[CURLOPT_PUT])) {
            $opts[CURLOPT_CUSTOMREQUEST] = "GET";
        }

        if ($use_cache) {
            $etag = self::get_cached_etag($url);
            if (!empty($etag)) {
                $this->send_headers[] = "If-None-Match: $etag";
            }
        }

        if (!empty($this->send_headers)) {
            $opts[CURLOPT_HTTPHEADER] = $this->send_headers;
        }

        $ch = curl_init();

        foreach ($opts as $k => $v) {
            if (LogSeverity::$is_debug) {
                if (is_bool($v)) {
                    hd_debug_print(HD::curlopt_to_string($k) . " ($k) = " . var_export($v, true));
                } else if (is_array($v)) {
                    hd_debug_print(HD::curlopt_to_string($k) . " ($k) = " . json_encode($v));
                } else {
                    hd_debug_print(HD::curlopt_to_string($k) . " ($k) = $v");
                }
            }
            curl_setopt($ch, $k, $v);
        }

        $start_tm = microtime(true);
        $content = curl_exec($ch);
        $execution_tm = microtime(true) - $start_tm;
        self::$error_no = curl_errno($ch);
        self::$error_desc = curl_error($ch);
        self::$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if (!is_null($fp)) {
            fclose($fp);
        }

        if (!empty(self::$http_response_headers) && LogSeverity::$is_debug) {
            hd_debug_print("---------  Response headers start ---------");
            foreach (self::$http_response_headers as $key => $header) {
                hd_debug_print("$key: $header");
            }
            hd_debug_print("---------   Response headers end  ---------");
        }

        if (self::$http_code < 200 || (self::$http_code >= 300 && self::$http_code != 301 && self::$http_code != 304)) {
            hd_debug_print("HTTP request failed (" . self::$http_code . ")");
            hd_debug_print("HTTP response " . $content);
            return false;
        }

        if (self::$error_no !== 0) {
            hd_debug_print(sprintf("CURL errno: %s (%s; HTTP error: %s;", self::$error_no, self::$error_desc, self::$http_code));
            return false;
        }

        if ($use_cache) {
            $new_etag = self::get_etag_header();
            if ($etag !== $new_etag) {
                hd_debug_print("Save new ETag ($new_etag) for: $url", true);
                self::set_cached_etag($url, $new_etag);
            }
        }

        if ($save_file === null) {
            hd_debug_print(sprintf("Return content: HTTP OK (%d, %d) in %.3fs", self::$http_code, strlen($content), $execution_tm), true);
        } else if ($save_file === false) {
            hd_debug_print(sprintf("Head response: HTTP OK (%d) in %.3fs", self::$http_code, $execution_tm), true);
        } else if (file_exists($save_file)) {
            hd_debug_print(sprintf("Save file: HTTP OK (%d, %d bytes) in %.3fs", self::$http_code, filesize($save_file), $execution_tm), true);
        } else {
            hd_debug_print(sprintf("HTTP code (%d) in %.3fs", self::$http_code, $execution_tm), true);
            hd_debug_print("Saved file '$save_file' is not exist!");
            return false;
        }

        return $save_file === null ? $content : true;
    }
}

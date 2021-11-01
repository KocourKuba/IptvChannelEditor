<?php
/** @noinspection PhpUndefinedClassInspection */

class HD
{
    public static function is_map($a)
    {
        return is_array($a) &&
            array_diff_key($a, array_keys(array_keys($a)));
    }

    ///////////////////////////////////////////////////////////////////////

    public static function has_attribute($obj, $n)
    {
        $arr = (array)$obj;
        return isset($arr[$n]);
    }

    ///////////////////////////////////////////////////////////////////////

    public static function get_map_element($map, $key)
    {
        return isset($map[$key]) ? $map[$key] : null;
    }

    ///////////////////////////////////////////////////////////////////////

    public static function starts_with($str, $pattern)
    {
        return strpos($str, $pattern) === 0;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function format_timestamp($ts, $fmt = null)
    {
        // NOTE: for some reason, explicit timezone is required for PHP
        // on Dune (no builtin timezone info?).

        if (is_null($fmt)) {
            $fmt = 'Y:m:d H:i:s';
        }

        $dt = new DateTime('@' . $ts);
        return $dt->format($fmt);
    }

    ///////////////////////////////////////////////////////////////////////

    public static function format_duration($msecs)
    {
        $n = (int)$msecs;

        if ($n <= 0 || strlen($msecs) <= 0) {
            return "--:--";
        }

        $n /= 1000;
        $hours = $n / 3600;
        $remainder = $n % 3600;
        $minutes = $remainder / 60;
        $seconds = $remainder % 60;

        if ((int)$hours > 0) {
            return sprintf("%d:%02d:%02d", $hours, $minutes, $seconds);
        }

        return sprintf("%02d:%02d", $minutes, $seconds);
    }

    public static function get_filesize_str($size)
    {
        if ($size < 1024) {
            $size_num = $size;
            $size_suf = "B";
        } else if ($size < 1048576) { // 1M
            $size_num = round($size / 1024, 2);
            $size_suf = "KiB";
        } else if ($size < 1073741824) { // 1G
            $size_num = round($size / 1048576, 2);
            $size_suf = "MiB";
        } else {
            $size_num = round($size / 1073741824, 2);
            $size_suf = "GiB";
        }
        return "$size_num $size_suf";
    }

    public static function get_storage_size($path, $arg = null)
    {
        $d[0] = disk_free_space($path);
        $d[1] = disk_total_space($path);
        foreach ($d as $bytes) {
            $si_prefix = array('B', 'KB', 'MB', 'GB', 'TB', 'EB', 'ZB', 'YB');
            $base = 1024;
            $class = min((int)log($bytes, $base), count($si_prefix) - 1);
            $size[] = sprintf('%1.2f', $bytes / pow($base, $class)) . ' ' . $si_prefix[$class];
        }

        if ($arg !== null) {
            $arr['str'] = $size[0] . '/' . $size[1];
            $arr['free_space'] = ($arg < $d[0]);
            return $arr;
        }
        return $size[0] . '/' . $size[1];
    }

    ///////////////////////////////////////////////////////////////////////

    public static function encode_user_data($a, $b = null)
    {
        if (is_array($a) && is_null($b)) {
            $media_url = '';
            $user_data = $a;
        } else {
            $media_url = $a;
            $user_data = $b;
        }

        if (!is_null($user_data)) {
            $media_url .= '||' . json_encode($user_data);
        }

        return $media_url;
    }

    ///////////////////////////////////////////////////////////////////////

    public static function decode_user_data($media_url_str, &$media_url, &$user_data)
    {
        $idx = strpos($media_url_str, '||');

        if ($idx === false) {
            $media_url = $media_url_str;
            $user_data = null;
            return;
        }

        $media_url = substr($media_url_str, 0, $idx);
        $user_data = json_decode(substr($media_url_str, $idx + 2), true);
    }

    ///////////////////////////////////////////////////////////////////////

    public static function create_regular_folder_range($items, $from_ndx = 0, $total = -1, $more_items_available = false)
    {
        if ($total === -1) {
            $total = $from_ndx + count($items);
        }

        if ($from_ndx >= $total) {
            $from_ndx = $total;
            $items = array();
        } else if ($from_ndx + count($items) > $total) {
            array_splice($items, $total - $from_ndx);
        }

        return array
        (
            PluginRegularFolderRange::total => (int)$total,
            PluginRegularFolderRange::more_items_available => $more_items_available,
            PluginRegularFolderRange::from_ndx => (int)$from_ndx,
            PluginRegularFolderRange::count => count($items),
            PluginRegularFolderRange::items => $items
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function http_get_document($url, $opts = null)
    {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 30);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);
        curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; rv:25.0) Gecko/20100101 Firefox/25.0");
        curl_setopt($ch, CURLOPT_ENCODING, 'gzip,deflate');
        curl_setopt($ch, CURLOPT_URL, $url);

        if (isset($opts)) {
            foreach ($opts as $k => $v) {
                curl_setopt($ch, $k, $v);
            }
        }

        hd_print("HTTP fetching '$url'");

        $content = curl_exec($ch);
        $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if ($content === false) {
            $err_msg = "HTTP error: $http_code (" . curl_error($ch) . ')';
            hd_print($err_msg);
            throw new Exception($err_msg);
        }

        if ($http_code !== 200) {
            $err_msg = "HTTP request failed ($http_code): " . self::http_status_code_to_string($http_code);
            hd_print($err_msg);
            throw new Exception($err_msg);
        }

        if (empty($content)) {
            $err_msg = "Empty content";
            hd_print($err_msg);
            throw new Exception($err_msg);
        }

        curl_close($ch);

        return $content;
    }

    public static function http_status_code_to_string($code){
        // Source: http://en.wikipedia.org/wiki/List_of_HTTP_status_codes

        switch( $code ){
            // 1xx Informational
            case 100: $string = 'Continue'; break;
            case 101: $string = 'Switching Protocols'; break;
            case 102: $string = 'Processing'; break; // WebDAV
            case 122: $string = 'Request-URI too long'; break; // Microsoft

            // 2xx Success
            case 200: $string = 'OK'; break;
            case 201: $string = 'Created'; break;
            case 202: $string = 'Accepted'; break;
            case 203: $string = 'Non-Authoritative Information'; break; // HTTP/1.1
            case 204: $string = 'No Content'; break;
            case 205: $string = 'Reset Content'; break;
            case 206: $string = 'Partial Content'; break;
            case 207: $string = 'Multi-Status'; break; // WebDAV

            // 3xx Redirection
            case 300: $string = 'Multiple Choices'; break;
            case 301: $string = 'Moved Permanently'; break;
            case 302: $string = 'Found'; break;
            case 303: $string = 'See Other'; break; //HTTP/1.1
            case 304: $string = 'Not Modified'; break;
            case 305: $string = 'Use Proxy'; break; // HTTP/1.1
            case 306: $string = 'Switch Proxy'; break; // Depreciated
            case 307: $string = 'Temporary Redirect'; break; // HTTP/1.1

            // 4xx Client Error
            case 400: $string = 'Bad Request'; break;
            case 401: $string = 'Unauthorized'; break;
            case 402: $string = 'Payment Required'; break;
            case 403: $string = 'Forbidden'; break;
            case 404: $string = 'Not Found'; break;
            case 405: $string = 'Method Not Allowed'; break;
            case 406: $string = 'Not Acceptable'; break;
            case 407: $string = 'Proxy Authentication Required'; break;
            case 408: $string = 'Request Timeout'; break;
            case 409: $string = 'Conflict'; break;
            case 410: $string = 'Gone'; break;
            case 411: $string = 'Length Required'; break;
            case 412: $string = 'Precondition Failed'; break;
            case 413: $string = 'Request Entity Too Large'; break;
            case 414: $string = 'Request-URI Too Long'; break;
            case 415: $string = 'Unsupported Media Type'; break;
            case 416: $string = 'Requested Range Not Satisfiable'; break;
            case 417: $string = 'Expectation Failed'; break;
            case 422: $string = 'Unprocessable Entity'; break; // WebDAV
            case 423: $string = 'Locked'; break; // WebDAV
            case 424: $string = 'Failed Dependency'; break; // WebDAV
            case 425: $string = 'Unordered Collection'; break; // WebDAV
            case 426: $string = 'Upgrade Required'; break;
            case 449: $string = 'Retry With'; break; // Microsoft
            case 450: $string = 'Blocked'; break; // Microsoft

            // 5xx Server Error
            case 500: $string = 'Internal Server Error'; break;
            case 501: $string = 'Not Implemented'; break;
            case 502: $string = 'Bad Gateway'; break;
            case 503: $string = 'Service Unavailable'; break;
            case 504: $string = 'Gateway Timeout'; break;
            case 505: $string = 'HTTP Version Not Supported'; break;
            case 506: $string = 'Variant Also Negotiates'; break;
            case 507: $string = 'Insufficient Storage'; break; // WebDAV
            case 509: $string = 'Bandwidth Limit Exceeded'; break; // Apache
            case 510: $string = 'Not Extended'; break;

            // Unknown code:
            default: $string = 'Unknown'; break;
        }
        return $string;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function http_post_document($url, $post_data)
    {
        return self::http_get_document($url,
            array
            (
                CURLOPT_POST => true,
                CURLOPT_POSTFIELDS => $post_data
            ));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function parse_xml_document($doc)
    {
        $xml = simplexml_load_string($doc);

        if ($xml === false) {
            hd_print("Error: can not parse XML document.");
            hd_print("XML-text: $doc.");
            throw new Exception('Illegal XML document');
        }

        return $xml;
    }

    /**
     * @throws Exception
     */
    public static function parse_xml_file($path)
    {
        $xml = simplexml_load_string(file_get_contents($path));

        if ($xml === false) {
            hd_print("Error: can not parse XML document.");
            hd_print("path to XML: $path");
            throw new Exception('Illegal XML document');
        }

        return $xml;
    }

    public static function parse_json_file($path, $to_array = true)
    {
        return json_decode(file_get_contents(
            $path,
            FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES),
            $to_array);
    }

    ///////////////////////////////////////////////////////////////////////

    public static function make_json_rpc_request($op_name, $params)
    {
        static $request_id = 0;

        return array
        (
            'jsonrpc' => '2.0',
            'id' => ++$request_id,
            'method' => $op_name,
            'params' => $params
        );
    }

    ///////////////////////////////////////////////////////////////////////////

    public static function get_mac_addr()
    {
        static $mac_addr = null;

        if (is_null($mac_addr)) {
            $mac_addr = shell_exec(
                'ifconfig  eth0 | head -1 | sed "s/^.*HWaddr //"');

            $mac_addr = trim($mac_addr);

            hd_print("MAC Address: '$mac_addr'");
        }

        return $mac_addr;
    }

    public static function get_ip_address()
    {
        static $ip_address = null;

        if (is_null($ip_address)) {
            $ip_address = trim(shell_exec('ifconfig eth0 | head -2 | tail -1 | sed "s/^.*inet addr:\([^ ]*\).*$/\1/"'));
        }

        return $ip_address;
    }

    ///////////////////////////////////////////////////////////////////////////

    private static $MONTHS = array(
        'January',
        'February',
        'March',
        'April',
        'May',
        'June',
        'July',
        'August',
        'September',
        'October',
        'November',
        'December',
    );

    public static function format_date_time_date($tm)
    {
        $lt = localtime($tm);
        $mon = self::$MONTHS[$lt[4]];
        return sprintf("%02d %s %04d", $lt[3], $mon, $lt[5] + 1900);
    }

    public static function format_date_time_time($tm, $with_sec = false)
    {
        $format = '%H:%M';
        if ($with_sec) {
            $format .= ':%S';
        }
        return strftime($format, $tm);
    }

    public static function print_backtrace()
    {
        hd_print('Back trace:');
        foreach (debug_backtrace() as $f) {
            hd_print('  - ' . $f['function'] . ' at ' . $f['file'] . ':' . $f['line']);
        }
    }

    public static function unescape_entity_string($raw_string)
    {
        $replace = array(
            "&nbsp;" => ' ',
            '&#39;'  => "'",
            '&gt;'   => ">",
            '&lt;'   => "<'>",
            '&apos;' => "'",
            '&quot;' => '"',
            '&amp;'  => '&',
            '&#196;' => 'Г„',
            '&#228;' => 'Г¤',
            '&#214;' => 'Г–',
            '&#220;' => 'Гњ',
            '&#223;' => 'Гџ',
            '&#246;' => 'Г¶',
            '&#252;' => 'Гј',
            '&#257;' => 'ā',
            '&#258;' => 'Ă',
            '&#268;' => 'Č',
            '&#326;' => 'ņ',
            '&#327;' => 'Ň',
            '&#363;' => 'ū',
            '&#362;' => 'Ū',
            '&#352;' => 'Š',
            '&#353;' => 'š',
            '&#382;' => 'ž',
            '&#275;' => 'ē',
            '&#276;' => 'Ĕ',
            '&#298;' => 'Ī',
            '&#299;' => 'ī',
            '&#291;' => 'ģ',
            '&#311;' => 'ķ',
            '&#316;' => 'ļ',
        );

        return str_replace(array_keys($replace), $replace, $raw_string);
    }

    public static function ArrayToStr($arrayItems)
    {
        $array = array();
        foreach ($arrayItems as $item) {
            if (!empty($item)) {
                $array[] = $item;
            }
        }

        return implode(", ", $array);
    }

    public static function get_items($path)
    {
        $full_path = self::get_data_path($path);
        return file_exists($full_path) ? unserialize(file_get_contents($full_path)) : array();
    }

    public static function put_items($path, $items)
    {
        file_put_contents(self::get_data_path($path), serialize($items));
    }

    public static function get_item($path)
    {
        $full_path = self::get_data_path($path);
        return file_exists($full_path) ? file_get_contents($full_path) : '';
    }

    public static function put_item($path, $item)
    {
        file_put_contents(self::get_data_path($path), $item);
    }

    public static function get_data_path($path = '')
    {
        if (!empty($path)) {
            $path = '/' . $path;
        }

        return DuneSystem::$properties['data_dir_path'] . $path;
    }

    public static function get_install_path($path = '')
    {
        if (!empty($path)) {
            $path = '/' . $path;
        }

        return DuneSystem::$properties['install_dir_path'] . $path;
    }

    public static function is_newer_versions()
    {
        $versions = self::get_firmware_version();

        return (isset($versions['rev_number']) && $versions['rev_number'] > 10);
    }

    public static function get_platform_kind()
    {
        static $result = null;

        if (is_null($result)) {
            $result = trim(shell_exec('grep "platform_kind" /tmp/run/versions.txt | sed "s/^.*= *//"'));
        }

        return $result;
    }

    public static function get_product_id()
    {
        static $result = null;

        if (is_null($result)) {
            $result = trim(shell_exec('grep "product_id:" /tmp/sysinfo.txt | sed "s/^.*: *//"'));
        }

        return $result;
    }

    public static function get_raw_firmware_version()
    {
        static $fw = null;

        if (is_null($fw)) {
            return trim(shell_exec('grep "firmware_version:" /tmp/sysinfo.txt | sed "s/^.*: *//"'));
        }

        return $fw;
    }

    public static function get_firmware_version()
    {
        static $result = null;

        if (is_null($result)) {
            preg_match_all('/^(\d*)_(\d*)_(\D*)(\d*)(.*)$/', self::get_raw_firmware_version(), $matches, PREG_SET_ORDER);
            $matches[0][5] = ltrim($matches[0][5], '_');
            $result = array_combine(array('string', 'build_date', 'build_number', 'rev_literal', 'rev_number', 'features'), $matches[0]);
        }

        return $result;
    }
}

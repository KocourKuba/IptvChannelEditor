<?php
///////////////////////////////////////////////////////////////////////////

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

    public static function format_timestamp($ts, $fmt = null)
    {
        // NOTE: for some reason, explicit timezone is required for PHP
        // on Dune (no builtin timezone info?).

        if (is_null($fmt))
            $fmt = 'Y:m:d H:i:s';

        $dt = new DateTime('@' . $ts);
        return $dt->format($fmt);
    }

    ///////////////////////////////////////////////////////////////////////

    public static function format_duration($msecs)
    {
        $n = intval($msecs);

        if (strlen($msecs) <= 0 || $n <= 0)
            return "--:--";

        $n = $n / 1000;
        $hours = $n / 3600;
        $remainder = $n % 3600;
        $minutes = $remainder / 60;
        $seconds = $remainder % 60;

        if (intval($hours) > 0) {
            return sprintf("%d:%02d:%02d", $hours, $minutes, $seconds);
        } else {
            return sprintf("%02d:%02d", $minutes, $seconds);
        }
    }

    ///////////////////////////////////////////////////////////////////////

    public static function encode_user_data($a, $b = null)
    {
        $media_url = null;
        $user_data = null;

        if (is_array($a) && is_null($b)) {
            $media_url = '';
            $user_data = $a;
        } else {
            $media_url = $a;
            $user_data = $b;
        }

        if (!is_null($user_data))
            $media_url .= '||' . json_encode($user_data);

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
        $user_data = json_decode(substr($media_url_str, $idx + 2));
    }

    ///////////////////////////////////////////////////////////////////////

    public static function create_regular_folder_range($items,
                                                       $from_ndx = 0, $total = -1, $more_items_available = false)
    {
        if ($total === -1)
            $total = $from_ndx + count($items);

        if ($from_ndx >= $total) {
            $from_ndx = $total;
            $items = array();
        } else if ($from_ndx + count($items) > $total) {
            array_splice($items, $total - $from_ndx);
        }

        return array
        (
            PluginRegularFolderRange::total => intval($total),
            PluginRegularFolderRange::more_items_available => $more_items_available,
            PluginRegularFolderRange::from_ndx => intval($from_ndx),
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
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 25);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_TIMEOUT, 25);
        curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; rv:25.0) Gecko/20100101 Firefox/25.0");
        curl_setopt($ch, CURLOPT_ENCODING, 1);
        curl_setopt($ch, CURLOPT_URL, $url);

        if (isset($opts)) {
            foreach ($opts as $k => $v)
                curl_setopt($ch, $k, $v);
        }

        hd_print("HTTP fetching '$url'...");

        $content = curl_exec($ch);
        $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if ($content === false) {
            $err_msg = "HTTP error: $http_code (" . curl_error($ch) . ')';
            hd_print($err_msg);
            throw new Exception($err_msg);
        }

        if ($http_code != 200) {
            $err_msg = "HTTP request failed ($http_code)";
            hd_print($err_msg);
            throw new Exception($err_msg);
        }

        hd_print("HTTP OK ($http_code)");

        curl_close($ch);

        return $content;
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

    ///////////////////////////////////////////////////////////////////////////

    // TODO: localization
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
        if ($with_sec)
            $format .= ':%S';
        return strftime($format, $tm);
    }

    public static function print_backtrace()
    {
        hd_print('Back trace:');
        foreach (debug_backtrace() as $f) {
            hd_print(
                '  - ' . $f['function'] .
                ' at ' . $f['file'] . ':' . $f['line']);
        }
    }

    /**
     * @param $url
     * @param $day_start_ts
     * @return array
     */
    public static function parse_epg_json($url, $day_start_ts)
    {
        $epg = array();
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);

        try {
            hd_print("epg uri: $url");
            $doc = HD::http_get_document($url);
        }
        catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        // stripe UTF8 BOM if exists
        $ch_data = json_decode(ltrim($doc, "\0xEF\0xBB\0xBF"));
        foreach ($ch_data->epg_data as $channel) {
            if ($channel->time >= $epg_date_start and $channel->time < $epg_date_end) {
                $epg[$channel->time]['title'] = HD::unescape_entity_string($channel->name);
                $epg[$channel->time]['desc'] = HD::unescape_entity_string($channel->descr);
            }
        }
        return $epg;
    }

    /**
     * @param $url
     * @param $epg_date
     * @return array
     */
    public static function parse_epg_html($url, $epg_date)
    {
        // html parse for tvguide.info
        // tvguide.info time in GMT+3 (moscow time)

        $epg = array();
        $e_time = strtotime("$epg_date, 0300 GMT+3");

        try {
            hd_print("epg uri: $url");
            $doc = HD::http_get_document($url);
        }
        catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        preg_match_all('|<div id="programm_text">(.*?)</div>|', $doc, $keywords);
        foreach ($keywords[1] as $qid) {
            $qq = strip_tags($qid);
            preg_match_all('|(\d\d:\d\d)&nbsp;(.*?)&nbsp;(.*)|', $qq, $keyw);
            $time = $keyw[1][0];
            $u_time = strtotime("$epg_date $time GMT+3");
            $last_time = ($u_time < $e_time) ? $u_time + 86400 : $u_time;
            $epg[$last_time]["title"] = HD::unescape_entity_string($keyw[2][0]);
            $epg[$last_time]["desc"] = HD::unescape_entity_string($keyw[3][0]);
        }

        return $epg;
    }

    /**
     * @param $url
     * @param $epg_id
     * @param $day_start_ts
     * @param $cache_dir
     * @return array
     */
    public static function parse_epg_xml($url, $epg_id, $day_start_ts, $cache_dir)
    {
        $epg = array();
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);

        try {
            // checks if epg already loaded
            preg_match('|^.*\/(.+)$|', $url, $match);
            $epgCacheFile = $cache_dir . $day_start_ts . '_' . $match[1];
            if (!file_exists($epgCacheFile)) {
                hd_print("epg uri: $url");
                $doc = HD::http_get_document($url);
                if(!file_put_contents($epgCacheFile, $doc)) {
                    hd_print("Writing to {$epgCacheFile} is not possible!");
                }
            }

            // parse
            $Parser = new EpgParser();
            $Parser->setFile($epgCacheFile);
            //$Parser->setTargetTimeZone('Europe/Berlin');
            $Parser->setChannelfilter($epg_id);
            $Parser->parseEpg();
            $epg_data = $Parser->getEpgData();
            if (empty($epg_data)){
                hd_print("No EPG data found");
            } else {
                foreach ($epg_data as $channel) {
                    if ($channel->time >= $epg_date_start and $channel->time < $epg_date_end) {
                        $epg[$channel->time]['title'] = HD::unescape_entity_string($channel->name);
                        $epg[$channel->time]['desc'] = HD::unescape_entity_string($channel->descr);
                    }
                }
            }
        }
        catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        return $epg;
    }

    public static function unescape_entity_string($raw_string)
    {
        $replace = array(
            '&#196;' => 'Г„',
            '&#228;' => 'Г¤',
            '&#214;' => 'Г–',
            '&#220;' => 'Гњ',
            '&#223;' => 'Гџ',
            '&#246;' => 'Г¶',
            '&#252;' => 'Гј',
            "&nbsp;" => ' ',
            '&#39;'  => "'",
            '&quot;' => '"',
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
}

///////////////////////////////////////////////////////////////////////////
?>

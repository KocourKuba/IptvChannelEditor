<?php
require_once 'utils.php';
require_once 'epg_xml_parser.php';

class EpgManager
{
    const EPG_CACHE_DIR_TEMPLATE  = '/tmp/%s_epg/';
    const EPG_CACHE_FILE_TEMPLATE = '/tmp/%s_epg/epg_channel_%s_%s';

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param $parser_params
     * @param IChannel $channel
     * @param $type
     * @param $day_start_ts
     * @param $cache_name
     * @return array
     * @throws Exception
     */
    public static function get_epg($parser_params, IChannel $channel, $type, $day_start_ts, $cache_name)
    {
        switch ($type)
        {
            case 'first':
                $epg_id = $channel->get_epg_id();
                break;
            case 'second':
                $epg_id = $channel->get_tvg_id();
                break;
            default:
                $epg_id = '';
        }

        if (empty($epg_id) || !isset($parser_params[$type]) || !isset($parser_params[$type]['epg_template'])) {
            throw new Exception("$type EPG not defined");
        }

        $params = $parser_params[$type];
        $cache_dir =  sprintf(EpgManager::EPG_CACHE_DIR_TEMPLATE, $cache_name);
        $cache_file = sprintf(EpgManager::EPG_CACHE_FILE_TEMPLATE, $cache_name, $channel->get_id(), $day_start_ts);

        if (file_exists($cache_file)) {
            hd_print("Load EPG from cache: $cache_file");
            $epg = unserialize(file_get_contents($cache_file));
        } else {
            $epg_date = gmdate("Y-m-d", $day_start_ts); // 'YYYYMMDD'
            $url = sprintf($params['epg_template'], str_replace(' ', '%20', $epg_id), $epg_date);
            hd_print("Fetching EPG for ID: '$epg_id' DATE: $epg_date");

            switch ($params['parser']) {
                case 'json':
                    $epg = self::get_epg_json($params, $url, $day_start_ts);
                    break;
                case 'xml':
                    $epg = self::get_epg_xml($params, $url, $day_start_ts, $epg_id, $cache_dir);
                    break;
                default:
                    $epg = array();
            }

            // sort epg by date
            $counts = count($epg);
            if ($counts > 0) {
                hd_print("Save $counts EPG entries to cache: $cache_file");

                if (!is_dir($cache_dir)) {
                    mkdir($cache_dir);
                }

                ksort($epg, SORT_NUMERIC);
                file_put_contents($cache_file, serialize($epg));
            }
        }

        return $epg;
    }

    /**
     * request server for epg and parse json response
     * @param $parser_params
     * @param $url
     * @param $day_start_ts
     * @return array
     */
    protected static function get_epg_json($parser_params, $url, $day_start_ts)
    {
        $epg = array();
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);
        // hd_print("epg start: $epg_date_start");
        // hd_print("epg end: $epg_date_end");

        try {
            $doc = HD::http_get_document($url);
            if (empty($doc)) {
                hd_print("Empty document returned.");
                return $epg;
            }
        }
        catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        // stripe UTF8 BOM if exists
        $ch_data = json_decode(ltrim($doc, "\0xEF\0xBB\0xBF"), true);
        $epg_root = $parser_params['epg_root'];
        // hd_print("json epg root: " . $parser_params['epg_root']);
        // hd_print("json start: " . $parser_params['start']);
        // hd_print("json end: " . $parser_params['end']);
        // hd_print("json title: " . $parser_params['title']);
        // hd_print("json desc: " . $parser_params['description']);

        if (!empty($epg_root) && isset($ch_data[$epg_root])) {
            // hd_print("use root: $epg_root");
            $data = $ch_data[$epg_root]; // sharvoz, edem, fox, itv
        } else {
            // hd_print("no root");
            $data = $ch_data; // sharaclub, no json root
        }

        // hd_print("total entries: " . count($data));
        // collect all program that starts after day start and before day end
        foreach ($data as $entry) {
            $program_start = $entry[$parser_params['start']];
            if ($epg_date_start <= $program_start && $program_start <= $epg_date_end) {
                $epg[$program_start]['end'] = $entry[$parser_params['end']];
                $epg[$program_start]['title'] = HD::unescape_entity_string($entry[$parser_params['title']]);
                $epg[$program_start]['desc'] = HD::unescape_entity_string($entry[$parser_params['description']]);
            }
        }
        return $epg;
    }

    /**
     * request server for XMLTV epg and parse xml or xml.gx response
     * @param $parser_params
     * @param $url
     * @param $day_start_ts
     * @param $epg_id
     * @param $cache_dir
     * @return array
     */
    protected  static function get_epg_xml($parser_params, $url, $day_start_ts, $epg_id, $cache_dir)
    {
        $epg = array();
        // time in UTC
        $epg_date_start = strtotime('-1 hour', $day_start_ts);
        $epg_date_end = strtotime('+1 day', $day_start_ts);

        try {
            // checks if epg already loaded
            preg_match('/^.*\/(.+)$/', $url, $match);
            $epgCacheFile = $cache_dir . $day_start_ts . '_' . $match[1];
            if (!file_exists($epgCacheFile)) {
                hd_print("epg uri: $url");
                $doc = HD::http_get_document($url);
                if(!file_put_contents($epgCacheFile, $doc)) {
                    hd_print("Writing to $epgCacheFile is not possible!");
                }
            }

            // parse
            $Parser = new EpgXmlParser();
            $Parser->setFile($epgCacheFile);
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
}
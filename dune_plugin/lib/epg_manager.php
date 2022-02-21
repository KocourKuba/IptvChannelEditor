<?php
require_once 'utils.php';
require_once 'epg_xml_parser.php';

class EpgManager
{
    const EPG_CACHE_DIR_TEMPLATE  = '/tmp/%s_epg/';
    const EPG_CACHE_FILE_TEMPLATE = '/tmp/%s_epg/epg_channel_%s_%s';

    public $config;

    public function __construct($config)
    {
        $this->config = $config;
    }

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param IChannel $channel
     * @param $type
     * @param $day_start_ts
     * @return array
     * @throws Exception
     */
    public function get_epg(IChannel $channel, $type, $day_start_ts, $plugin_cookies)
    {
        switch ($type)
        {
            case 'first':
                $epg_id = str_replace(' ', '%20', $channel->get_epg_id());
                break;
            case 'second':
                $epg_id = str_replace(' ', '%20', $channel->get_tvg_id());
                break;
            default:
                $epg_id = '';
        }

        if (empty($epg_id)) {
            hd_print("EPG: $epg_id not defined");
            throw new Exception("EPG: $epg_id not defined");
        }

        $config = $this->config;
        $epg_url = $config->get_epg_url($type, $epg_id, $day_start_ts, $plugin_cookies);
        if (empty($epg_url)) {
            hd_print("$type EPG url not defined");
            throw new Exception("$type EPG url not defined");
        }


        $cache_dir =  sprintf(self::EPG_CACHE_DIR_TEMPLATE, $config->PLUGIN_SHORT_NAME);
        $cache_file = sprintf(self::EPG_CACHE_FILE_TEMPLATE, $config->PLUGIN_SHORT_NAME, $channel->get_id(), $day_start_ts);

        $parser_params = $config->get_epg_params();
        $params = $parser_params[$type];

        if (file_exists($cache_file)) {
            hd_print("Load EPG from cache: $cache_file");
            $epg = unserialize(file_get_contents($cache_file));
        } else {
            switch ($params['parser']) {
                case 'json':
                    $epg = self::get_epg_json($epg_url, $params, $day_start_ts);
                    break;
                case 'xml':
                    $epg = self::get_epg_xml($epg_url, $day_start_ts, $epg_id, $cache_dir);
                    break;
                default:
                    $epg = array();
            }

            // sort epg by date
            $counts = count($epg);
            if ($counts > 0) {
                hd_print("Save $counts EPG entries to cache: $cache_file");
                if (!is_dir($cache_dir) && !mkdir($cache_dir) && !is_dir($cache_dir)) {
                    hd_print("Directory '$cache_dir' was not created");
                }

                ksort($epg, SORT_NUMERIC);
                file_put_contents($cache_file, serialize($epg));
            }
        }

        return $epg;
    }

    /**
     * request server for epg and parse json response
     * @param $url
     * @param $parser_params
     * @param $day_start_ts
     * @return array
     */
    protected static function get_epg_json($url, $parser_params, $day_start_ts)
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
            hd_print("http exception: " . $ex->getMessage());
            return $epg;
        }

        // stripe UTF8 BOM if exists
        $ch_data = json_decode(ltrim($doc, "\xEF\xBB\xBF"), true);
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

        hd_print("total entries: " . count($data));
        // collect all program that starts after day start and before day end
        foreach ($data as $entry) {
            $program_start = $entry[$parser_params['start']];
            if ($epg_date_start <= $program_start && $program_start <= $epg_date_end) {
                if ($parser_params['use_duration']) {
                    $epg[$program_start]['end'] = $program_start + (int)$entry[$parser_params['end']];
                } else {
                    $epg[$program_start]['end'] = $entry[$parser_params['end']];
                }
                $epg[$program_start]['title'] = HD::unescape_entity_string($entry[$parser_params['title']]);
                $epg[$program_start]['desc'] = HD::unescape_entity_string($entry[$parser_params['description']]);
            }
        }
        return $epg;
    }

    /**
     * request server for XMLTV epg and parse xml or xml.gx response
     * @param $url
     * @param $day_start_ts
     * @param $epg_id
     * @param $cache_dir
     * @return array
     */
    protected static function get_epg_xml($url, $day_start_ts, $epg_id, $cache_dir)
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
                    if ($channel->time >= $epg_date_start && $channel->time < $epg_date_end) {
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
<?php
require_once 'hd.php';
require_once 'v32.php';
require_once 'epg_xml_parser.php';

class Epg_Manager
{
    const EPG_CACHE_DIR_TEMPLATE = '/tmp/%s_epg/';
    const EPG_CACHE_FILE_TEMPLATE = '/tmp/%s_epg/epg_channel_%s_%s';

    /**
     * @var Default_Config
     */
    public $config;

    /**
     * @param Default_Config $config
     */
    public function __construct(Default_Config $config)
    {
        $this->config = $config;
    }

    /**
     * try to load epg from cache otherwise request it from server
     * store parsed response to the cache
     * @param Channel $channel
     * @param string $type
     * @param int $day_start_ts
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_epg(Channel $channel, $type, $day_start_ts, $plugin_cookies)
    {
        $params = $this->config->get_epg_params($type);

        if (empty($params['epg_url'])) {
            hd_print("$type EPG url not defined");
            throw new Exception("$type EPG url not defined");
        }

        switch ($type) {
            case 'first':
                $epg_id = $channel->get_epg_id();
                break;
            case 'second':
                $epg_id = $channel->get_tvg_id();
                break;
            default:
                $epg_id = '';
        }

        if (empty($epg_id)) {
            hd_print("EPG: $epg_id not defined");
            throw new Exception("EPG: $epg_id not defined");
        }

        if ($params['use_epg_mapper']) {
            $mapper = $params['tvg_id_mapper'];
            if (array_key_exists($epg_id, $mapper)) {
                hd_print("EPG id replaced: $epg_id -> " . $mapper[$epg_id]);
                $epg_id = $mapper[$epg_id];
            }
        } else if ($params['use_epg_hash']) {
            $xx_hash = new V32();
            $epg_id = $xx_hash->hash($epg_id);
        } else {
            $epg_id = str_replace(' ', '%20', $epg_id);
        }

        $epg_date = gmdate($params['date_format'], $day_start_ts);
        $epg_url = str_replace(
            array('{TOKEN}', '{CHANNEL}', '{DATE}', '{TIME}'),
            array(isset($plugin_cookies->token) ? $plugin_cookies->token : '', $epg_id, $epg_date, $day_start_ts),
            $params['epg_url']);

        if (isset($plugin_cookies->use_epg_proxy)
            && $plugin_cookies->use_epg_proxy === 'yes'
            && $this->config->get_feature(PROXIED_EPG) === true) {

            $epg_url = str_replace('epg.ott-play.com', 'epg.esalecrm.net', $epg_url);
        }

        hd_print("Fetching EPG for ID: '$epg_id' DATE: $epg_date");

        $cache_dir = sprintf(self::EPG_CACHE_DIR_TEMPLATE, $this->config->PLUGIN_SHORT_NAME);
        $cache_file = sprintf(self::EPG_CACHE_FILE_TEMPLATE, $this->config->PLUGIN_SHORT_NAME, $channel->get_id(), $day_start_ts);

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
     * @param string $url
     * @param array $parser_params
     * @param int $day_start_ts
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
        } catch (Exception $ex) {
            hd_print("http exception: " . $ex->getMessage());
            return $epg;
        }

        // stripe UTF8 BOM if exists
        $ch_data = json_decode(ltrim($doc, "\xEF\xBB\xBF"), true);
        if (!empty($parser_params['epg_root'])) {
            foreach (explode('|', $parser_params['epg_root']) as $level) {
                $epg_root = $level;
                $ch_data = $ch_data[$epg_root];
            }
        }
        // hd_print("json epg root: " . $parser_params['epg_root']);
        // hd_print("json start: " . $parser_params['start']);
        // hd_print("json end: " . $parser_params['end']);
        // hd_print("json title: " . $parser_params['title']);
        // hd_print("json desc: " . $parser_params['description']);

        hd_print("total entries: " . count($ch_data));
        // collect all program that starts after day start and before day end
        foreach ($ch_data as $entry) {
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
     * @param string $url
     * @param int $day_start_ts
     * @param string $epg_id
     * @param string $cache_dir
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
                if (!file_put_contents($epgCacheFile, $doc)) {
                    hd_print("Writing to $epgCacheFile is not possible!");
                }
            }

            // parse
            $Parser = new Epg_Xml_Parser();
            $Parser->setFile($epgCacheFile);
            $Parser->setChannelfilter($epg_id);
            $Parser->parseEpg();
            $epg_data = $Parser->getEpgData();
            if (empty($epg_data)) {
                hd_print("No EPG data found");
            } else {
                foreach ($epg_data as $channel) {
                    if ($channel->time >= $epg_date_start && $channel->time < $epg_date_end) {
                        $epg[$channel->time]['title'] = HD::unescape_entity_string($channel->name);
                        $epg[$channel->time]['desc'] = HD::unescape_entity_string($channel->descr);
                    }
                }
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return $epg;
        }

        return $epg;
    }
}
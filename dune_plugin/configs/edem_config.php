<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'OTT_KEY';
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS');
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = true;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[VOD_PORTAL_SUPPORTED] = true;
        static::$FEATURES[VOD_LAZY_LOAD] = true;
    }

    public static function ParsePortalUrl($url, $plugin_cookies)
    {
        if (preg_match('|^portal::\[key:([^]]+)\](.+)$|', $url, $matches)) {
            $plugin_cookies->vkey = $matches[1];
            $plugin_cookies->vportal = $matches[2];
            return true;
        }

        return false;
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();
        // hd_print("Stream url:  " . $url);

        $subdomain = empty($plugin_cookies->subdomain_local) ? $plugin_cookies->subdomain : $plugin_cookies->subdomain_local;
        $token = empty($plugin_cookies->ott_key_local) ? $plugin_cookies->ott_key : $plugin_cookies->ott_key_local;
        if (empty($subdomain) || empty($token)) {
            hd_print("TransformStreamUrl: parameters for {$channel->get_channel_id()} not defined!");
        } else {
            // substitute subdomain token parameters
            $url = str_replace(
                array('{DOMAIN}', '{TOKEN}'),
                array($subdomain, $token),
                $url);
        }

        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account " . static::$PLUGIN_SHOW_NAME);

        return true;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        return array();
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            return sprintf('http://epg.ott-play.com/edem/epg/%s.json', $id);
        }

        return null;
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        hd_print("TryLoadMovie: $movie_id");

        $movie = new Movie($movie_id);

        $movieData = self::make_json_request($plugin_cookies,
            array('cmd' => "flick", 'fid' => (int)$movie_id, 'offset'=> 0,'limit' => 0));

        if ($movieData === false) {
            return $movie;
        }

        $movie->set_data(
            $movieData->title,// caption,
            '',// caption_original,
            $movieData->description,// description,
            $movieData->img,// poster_url,
            $movieData->duration,// length,
            $movieData->year,// year,
            '',// director,
            '',// scenario,
            '',// actors,
            '',// genres,
            '',// rate_imdb,
            '',// rate_kinopoisk,
            $movieData->agelimit,// rate_mpaa,
            '',// country,
            ''// budget
        );

        if ($movieData->type !== 'multistream') {
            //hd_print("movie playback_url: $movieData->url");
            $movie->add_series_data($movie_id, $movieData->title, $movieData->url, true);
            return $movie;
        }

        // collect series
        foreach ($movieData->items as $item) {
            // hd_print("episode playback_url: $episode->url");
            $movie->add_series_data($item->fid, $item->title, $item->url, true);
        }

        return $movie;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        hd_print("fetch_vod_categories");

        $doc = self::make_json_request($plugin_cookies);
        if ($doc === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        foreach ($doc->items as $node) {
            $cat = new StarnetVodCategory((string)$node->request->fid, (string)$node->title);
            $category_list[] = $cat;
            $category_index[$cat->get_id()] = $cat;
        }

        $filters = array();
        foreach ($doc->controls->filters as $filter) {
            $title = $filter->title;
            foreach ($filter->items as $item) {
                $filters[$title][] = $item;
            }
        }

        self::set_filters($filters);

        hd_print("Categories read: " . count($category_list));
        hd_print("Filters count: " . count($filters));
    }

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList $keyword");
        $searchRes = self::make_json_request($plugin_cookies,
            array('cmd' => "search", 'query' => $keyword));

        return $searchRes === false ? array() : self::CollectSearchResult('search', $searchRes);
    }

    public static function getFilterList($params, $plugin_cookies)
    {
        hd_print("getFilterList: $params");
        return array();
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($idx, $plugin_cookies)
    {
        $val = static::get_next_page($idx, 0);
        hd_print("getVideoList: $idx, $val");

        $categories = self::make_json_request($plugin_cookies,
            array('cmd' => "flicks", 'fid' => (int)$idx, 'offset' => (int)$val, 'limit' => 0));

        return $categories === false ? array() : self::CollectSearchResult($idx, $categories);
    }

    /**
     * @throws Exception
     */
    protected static function CollectSearchResult($idx, $json)
    {
        hd_print("CollectSearchResult: $idx");
        $movies = array();

        foreach ($json->items as $entry) {
            if ($entry->type === 'next') {
                self::get_next_page($idx, $entry->request->offset);
            } else {
                $movie = new ShortMovie($entry->request->fid, $entry->title, $entry->img);
                $movie->info = "$entry->title|Год: $entry->year|Рейтинг: $entry->agelimit";
                $movies[] = $movie;
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    protected static function make_json_request($plugin_cookies, $params = null, $to_array = false, $save_path = null)
    {
        if (empty($plugin_cookies->vportal) || empty($plugin_cookies->vkey)) {
            return false;
        }

        if ($params !== null) {
            $pairs = $params;
        }

        // fill default params
        $pairs['key'] = $plugin_cookies->vkey;
        $pairs['mac'] = "000000000000";
        $pairs['app'] = "edem_dune_plugin." . static::$PLUGIN_VERSION;

        $curl_opt = array
        (
            CURLOPT_HTTPHEADER => array("Content-Type: application/json"),
            CURLOPT_POST => true,
            CURLOPT_POSTFIELDS => json_encode($pairs)
        );

        // hd_print("post_data: " . json_encode($pairs));

        return HD::LoadAndStoreJson($plugin_cookies->vportal, $to_array, $save_path, $curl_opt);
    }

    public static function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        static::$movie_counter[$key] += $val;
    }
}

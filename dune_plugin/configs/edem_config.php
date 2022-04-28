<?php
require_once 'default_config.php';

class EdemPluginConfig extends Default_Config
{
    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'OTT_KEY');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8');
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(VOD_PORTAL_SUPPORTED, true);
        $this->set_feature(VOD_LAZY_LOAD, true);

        $this->set_epg_param('first','epg_url','http://technic.cf/epg-it999/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('first','epg_root', 'data');
        $this->set_epg_param('first','epg_start', 'begin');
        $this->set_epg_param('first','epg_end', 'end');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_date_format', 'Y.m.d');
    }

    /**
     * @param array &$defs
     * @param Starnet_Filter_Screen $parent
     * @param int $initial
     * @return bool
     */
    public function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        $filters = array("years", "genre");
        hd_print("AddFilterUI: $initial");
        $added = false;
        foreach ($filters as $name) {
            $filter = $this->get_filter($name);
            if ($filter === null) {
                hd_print("AddFilterUI: no filters with '$name'");
                continue;
            }

            $values = $filter['values'];
            if (empty($values)) {
                hd_print("AddFilterUI: no filters values for '$name'");
                continue;
            }

            $idx = $initial;
            if ($initial !== -1) {
                $pairs = explode(" ", $initial);
                foreach ($pairs as $pair) {
                    if (strpos($pair, $name . ":") !== false && preg_match("|^$name:(.+)|", $pair, $m)) {
                        $idx = array_search($m[1], $values) ?: -1;
                        break;
                    }
                }
            }

            Control_Factory::add_combobox($defs, $parent, null, $name,
                $filter['title'], $idx, $values, 600, true);

            Control_Factory::add_vgap($defs, 30);
            $added = true;
        }

        return $added;
    }

    /**
     * @param $user_input
     * @return string
     */
    public function CompileSaveFilterItem($user_input)
    {
        $filters = array("years", "genre");
        $compiled_string = "";
        foreach ($filters as $name) {
            $filter = $this->get_filter($name);
            if ($filter !== null && $user_input->{$name} !== -1) {
                if (!empty($compiled_string)) {
                    $compiled_string .= " ";
                }

                $compiled_string .= $name . ":" . $filter['values'][$user_input->{$name}];
            }
        }

        return $compiled_string;
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
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

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account $this->PLUGIN_SHOW_NAME");

        return true;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        return array();
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        hd_print("TryLoadMovie: $movie_id");
        $movie = new Movie($movie_id);
        $movieData = self::make_json_request($plugin_cookies,
            array('cmd' => "flick", 'fid' => (int)$movie_id, 'offset'=> 0,'limit' => 0));

        if ($movieData === false) {
            hd_print("TryLoadMovie: failed to load movie: $movie_id");
            return $movie;
        }

        if ($movieData->type === 'multistream') {
            // collect series
            //hd_print("movie: $movie_id \"$movieData->title\" contains " . count((array)$movieData->items) . " series");
            foreach ($movieData->items as $item) {
                //hd_print("Try load episode $item->fid playback_url: $item->url");
                $episodeData = self::make_json_request($plugin_cookies,
                    array('cmd' => "flick", 'fid' => (int)$item->fid, 'offset'=> 0,'limit' => 0));

                if (!isset($episodeData->variants) || count((array)$episodeData->variants) < 2) {
                    //hd_print("no variants for $item->fid");
                    $movie->add_series_data($item->fid, $item->title, $item->url);
                } else {
                    $variants_data = (array)$episodeData->variants;
                    //hd_print("episode $item->fid contains " . count($variants_data) . " variants");
                    // collect quality variants for single movie
                    $variants = array();
                    foreach ($variants_data as $key => $url) {
                        //hd_print("variant $key playback_url: $url");
                        $variants[] = new Movie_Variant($item->fid . "_" . $key, "Quality: $key", $url);
                    }

                    $series = new Movie_Series($item->fid);
                    $series->name = (string)$item->title;
                    $series->playback_url = (string)$episodeData->url;
                    $series->variants = $variants;
                    $movie->series_list[$item->fid] = $series;
                }
            }
        } else if (isset($movieData->variants) || count((array)$movieData->variants) < 2) {
            $variants_data = (array)$movieData->variants;
            //hd_print("movie $movie_id \"$movieData->title\" contains " . count($variants_data) . " variants");
            // collect quality variants for single movie
            $variants = array();
            foreach ($variants_data as $key => $url) {
                //hd_print("variant $key playback_url: $url");
                $variants[] = new Movie_Variant($movie_id . "_" . $key, "Quality: $key", $url);
            }

            $series = new Movie_Series($movie_id);
            $series->name = (string)$movieData->title;
            $series->variants = $variants;
            $movie->series_list[$movie_id] = $series;
        } else {
            $movie->add_series_data($movie_id, $movieData->title, $movieData->url);
        }

        $movie->set_data(
            $movieData->title,// caption,
            '',// caption_original,
            isset($movieData->description) ? $movieData->description : '',// description,
            isset($movieData->img) ? $movieData->img : '',// poster_url,
            isset($movieData->duration) ? $movieData->duration : '',// length,
            isset($movieData->year) ? $movieData->year : '',// year,
            '',// director,
            '',// scenario,
            '',// actors,
            '',// genres,
            '',// rate_imdb,
            '',// rate_kinopoisk,
            isset($movieData->agelimit) ? $movieData->agelimit : '',// rate_mpaa,
            '',// country,
            ''// budget
        );

        return $movie;
    }

    /**
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        //hd_print("fetch_vod_categories");

        $doc = self::make_json_request($plugin_cookies, null, true);
        if ($doc === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        foreach ($doc['items'] as $node) {
            $cat = new Starnet_Vod_Category((string)$node['request']['fid'], (string)$node['title']);
            $category_list[] = $cat;
            $category_index[$cat->get_id()] = $cat;
        }

        $filters = array();
        foreach ($doc['controls']['filters'] as $filter) {
            $first = reset($filter['items']);
            $key = key(array_diff_key($first['request'], array('filter' => 'on')));
            $filters[$key] = array('title' => $filter['title'], 'values' => array());
            $filters[$key]['values'][-1] = 'Нет';
            foreach ($filter['items'] as $item) {
                $val = $item['request'][$key];
                $filters[$key]['values'][$val] = $item['title'];
            }
        }

        $this->set_filters($filters);

        hd_print("Categories read: " . count($category_list));
        hd_print("Filters count: " . count($filters));
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList $keyword");
        $searchRes = self::make_json_request($plugin_cookies,
            array('cmd' => "search", 'query' => $keyword));

        return $searchRes === false ? array() : $this->CollectSearchResult($keyword, $searchRes);
    }

    /**
     * @param string $params
     * @param $plugin_cookies
     * @return array|false
     * @throws Exception
     */
    public function getFilterList($params, $plugin_cookies)
    {
        hd_print("getFilterList: $params");
        $pairs = explode(" ", $params);
        $post_params = array();
        foreach ($pairs as $pair) {
            if (preg_match("|^(.+):(.+)$|", $pair, $m)) {
                $filter = $this->get_filter($m[1]);
                if ($filter !== null && !empty($filter['values'])) {
                    $item_idx = array_search($m[2], $filter['values']);
                    if ($item_idx !== false && $item_idx !== -1) {
                        if ($m[1] === 'years') {
                            $post_params[$m[1]] = (string)$item_idx;
                        } else {
                            $post_params[$m[1]] = (int)$item_idx;
                        }
                    }
                }
            }
        }

        if (empty($post_params)) {
            return false;
        }

        $post_params['filter'] = 'on';
        $post_params['offset'] = $this->get_next_page($params, 0);
        $filterRes = self::make_json_request($plugin_cookies, $post_params);

        return $filterRes === false ? array() : $this->CollectSearchResult($params, $filterRes);
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $val = $this->get_next_page($query_id, 0);
        hd_print("getVideoList: $query_id, $val");

        $categories = self::make_json_request($plugin_cookies,
            array('cmd' => "flicks", 'fid' => (int)$query_id, 'offset' => $val, 'limit' => 0));

        return $categories === false ? array() : $this->CollectSearchResult($query_id, $categories);
    }

    /**
     * @param string $query_id
     * @param $json
     * @return array
     */
    protected function CollectSearchResult($query_id, $json)
    {
        // hd_print("CollectSearchResult: $query_id");
        $movies = array();

        $current_offset = $this->get_next_page($query_id, 0);
        if ($current_offset < 0)
            return $movies;

        foreach ($json->items as $entry) {
            if ($entry->type === 'next') {
                $this->get_next_page($query_id, $entry->request->offset);
            } else {
                $movie = new Short_Movie($entry->request->fid, $entry->title, $entry->img);
                $movie->info = "$entry->title|Год: $entry->year|Рейтинг: $entry->agelimit";
                $movies[] = $movie;
            }
        }
        if ($current_offset === $this->get_next_page($query_id, 0)) {
            $this->set_next_page($query_id, -1);
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param $plugin_cookies
     * @param array|null $params
     * @param bool $to_array
     * @param string|null $save_path
     * @return false|mixed
     */
    protected static function make_json_request($plugin_cookies, $params = null, $to_array = false, $save_path = null)
    {
        $mediateka = '';
        if (isset($plugin_cookies->mediateka_local)) {
            $mediateka = $plugin_cookies->mediateka_local;
        }

        if (empty($mediateka) && isset($plugin_cookies->mediateka)) {
            $mediateka = $plugin_cookies->mediateka;
        }

        if (empty($mediateka)
            || !preg_match('|^portal::\[key:([^]]+)\](.+)$|', $mediateka, $matches)) {
            hd_print("incorrect or empty VPortal key");
            return false;
        }

        list(, $key, $url) = $matches;

        $pairs = array();
        if ($params !== null) {
            $pairs = $params;
        }

        // fill default params
        $pairs['key'] = $key;
        $pairs['mac'] = "000000000000"; // dummy
        $pairs['app'] = "IPTV_ChannelEditor_edem_dune_plugin";

        $curl_opt = array
        (
            CURLOPT_HTTPHEADER => array("Content-Type: application/json"),
            CURLOPT_POST => true,
            CURLOPT_POSTFIELDS => json_encode($pairs)
        );

        // hd_print("post_data: " . json_encode($pairs));

        return HD::LoadAndStoreJson($url, $to_array, $save_path, $curl_opt);
    }

    /**
     * @param string $key
     * @param int $val
     */
    public function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, $this->movie_counter)) {
            $this->movie_counter[$key] = 0;
        }

        $this->movie_counter[$key] += $val;
    }
}

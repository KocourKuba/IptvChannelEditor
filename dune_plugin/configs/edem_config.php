﻿<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    const API_URL = 'http://technic.cf/epg-it999';

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

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'data';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'begin';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'end';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'description';
        static::$EPG_PARSER_PARAMS['first']['date_format'] = 'Y.m.d';
    }

    public function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        $filters = array("years", "genre");
        hd_print("AddFilterUI: $initial");
        $added = false;
        foreach ($filters as $name) {
            $filter = static::get_filter($name);
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

            ControlFactory::add_combobox($defs, $parent, null, $name,
                $filter['title'], $idx, $values, 600, true);

            ControlFactory::add_vgap($defs, 30);
            $added = true;
        }

        return $added;
    }

    public static function CompileSaveFilterItem($user_input)
    {
        $filters = array("years", "genre");
        $compiled_string = "";
        foreach ($filters as $name) {
            $filter = static::get_filter($name);
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

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
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
        /*
        hd_print("Collect information from account $this->PLUGIN_SHOW_NAME");
        static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper'] = HD::MapTvgID(self::API_URL . '/channels');
        hd_print("TVG ID Mapped: " . count(static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper']));
        */
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

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            $epg_date = gmdate(static::$EPG_PARSER_PARAMS['first']['date_format'], $day_start_ts);
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('%s/epg_day?id=%s&day=%s', self::API_URL, $id, $epg_date); // epg_id date(Y.m.d)
        }

        return null;
    }

    /**
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
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

        if ($movieData->type === 'multistream') {
            // collect series
            foreach ($movieData->items as $item) {
                //hd_print("episode playback_url: $item->url");
                $movie->add_series_data($item->fid, $item->title, $item->url, true);
            }
            return $movie;
        }

        //hd_print("movie playback_url: \"$movieData->title\" $movieData->url");
        $variants = (array)$movieData->variants;
        //hd_print("playback_url variants: " . count($variants));
        if (count($variants) > 1) {
            foreach ($variants as $key => $item) {
                //hd_print("variants playback_url: $key : $item");
                $movie->add_series_data($movie_id . "_$key", $key, $item, true);
            }
        } else {
            $movie->add_series_data($movie_id, $movieData->title, $movieData->url, true);
        }

        return $movie;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        hd_print("fetch_vod_categories");

        $doc = self::make_json_request($plugin_cookies, null, true);
        if ($doc === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        foreach ($doc['items'] as $node) {
            $cat = new StarnetVodCategory((string)$node['request']['fid'], (string)$node['title']);
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

        self::set_filters($filters);

        hd_print("Categories read: " . count($category_list));
        hd_print("Filters count: " . count($filters));
    }

    /**
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
     * @throws Exception
     */
    public function getFilterList($params, $plugin_cookies)
    {
        hd_print("getFilterList: $params");
        $pairs = explode(" ", $params);
        $post_params = array();
        foreach ($pairs as $pair) {
            if (preg_match("|^(.+):(.+)$|", $pair, $m)) {
                $filter = static::get_filter($m[1]);
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
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $val = $this->get_next_page($query_id, 0);
        hd_print("getVideoList: $query_id, $val");

        $categories = self::make_json_request($plugin_cookies,
            array('cmd' => "flicks", 'fid' => (int)$query_id, 'offset' => (int)$val, 'limit' => 0));

        return $categories === false ? array() : $this->CollectSearchResult($query_id, $categories);
    }

    /**
     * @throws Exception
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
                $movie = new ShortMovie($entry->request->fid, $entry->title, $entry->img);
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

    public static function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        static::$movie_counter[$key] += $val;
    }
}

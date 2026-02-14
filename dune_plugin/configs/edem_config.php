<?php
require_once 'lib/default_config.php';

class edem_config extends default_config
{
    /**
     * @return string
     */
    public function get_ott_key()
    {
        return isset($this->embedded_account->ott_key) ? $this->embedded_account->ott_key : $this->plugin->get_credentials(Ext_Params::M_OTT_KEY);
    }


    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);
        $jsonData = $this->make_json_request(array('cmd' => "flick", 'fid' => (int)$movie_id, 'offset' => 0, 'limit' => 0));

        if ($jsonData === false) {
            hd_debug_print("failed to load movie: $movie_id");
            return null;
        }

        $movie = new Movie($movie_id, $this->plugin);
        $qualities_str = '';
        $type = safe_get_value($jsonData, 'type');
        if ($type === 'stream') {
            $movie_series = new Movie_Series($movie_id, $jsonData->title, $jsonData->url);
            if (isset($jsonData->variants) && count((array)$jsonData->variants) === 1) {
                $movie_series->description = $movie->to_string(key($jsonData->variants));
            } else {
                $variants_data = (array)$jsonData->variants;
                foreach ($variants_data as $key => $url) {
                    $movie_series->qualities[$key] = new Movie_Variant($movie_id . "_" . $key, $key, $url);
                    if (!empty($qualities_str) && $key !== 'auto') {
                        $qualities_str .= ",";
                    }
                    $qualities_str .= ($key === 'auto' ? '' : $key);
                }

                $qualities_str = rtrim($qualities_str, ' ,\0');
                $movie_series->description = TR::load('vod_screen_quality') . "|$qualities_str";
            }
            $movie->add_series_data($movie_series);
        } else if ($type === 'multistream') {
            // collect series
            foreach ($jsonData->items as $item) {
                $episodeData = $this->make_json_request(array('cmd' => "flick", 'fid' => (int)$item->fid, 'offset' => 0, 'limit' => 0));

                $movie_series = new Movie_Series($item->fid, $item->title, $item->url);
                if (isset($episodeData->variants) && count((array)$episodeData->variants) === 1) {
                    $movie_series->description = $movie->to_string(key($episodeData->variants));
                } else {
                    $variants_data = (array)$episodeData->variants;
                    $qualities_str = '';
                    foreach ($variants_data as $key => $url) {
                        $movie_series->qualities[(string)$key] = new Movie_Variant($item->fid . "_" . $key, $key, $url);
                        if (!empty($qualities_str) && $key !== 'auto') {
                            $qualities_str .= ",";
                        }
                        $qualities_str .= ($key === 'auto' ? '' : $key);
                    }

                    $movie_series->description = TR::load('vod_screen_quality') . "|" . rtrim($qualities_str, ' ,\0');
                }
                $movie->add_series_data($movie_series);
            }
        } else {
            hd_debug_print("Unsupported type: '$type' for '$movie_id'");
            return null;
        }

        $rate_details = array();

        $age = safe_get_value($jsonData, 'agelimit');
        if (!empty($age)) {
            $rate_details[TR::t('vod_screen_age_limit')] = "$age+";
        }

        if (safe_get_value($jsonData, 'fhd', false)) {
            $rate_details['Full HD'] = TR::load('yes');
        }

        if (safe_get_value($jsonData, '4k', false)) {
            $rate_details['4K'] = TR::load('yes');
        }

        if (safe_get_value($jsonData, 'hdr', false)) {
            $rate_details['HDR'] = TR::load('yes');
        }

        $details = array();
        if (!empty($qualities_str)) {
            $details[TR::t('vod_screen_quality')] = $qualities_str;
        }

        $movie->set_data(
            safe_get_value($jsonData, 'title', TR::t('no_title')),      // caption
            '',         // caption_original
            safe_get_value($jsonData, 'description', ''),  // description
            safe_get_value($jsonData, 'img', ''),  // poster_url
            safe_get_value($jsonData, 'duration', ''),    // length
            safe_get_value($jsonData, 'year', ''),    // year
            '',          // director
            '',          // scenario
            '',            // actors
            '',            // genres
            '',            // rate_imdb,
            '',         // rate_kinopoisk
            '',            // rate_mpaa
            '',              // country
            '',              // budget
            $details, // details
            $rate_details // rate details
        );

        return $movie;
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        $doc = $this->make_json_request();
        if ($doc === false) {
            return false;
        }

        $category_list = array();
        $category_index = array();

        if (isset($doc->items)) {
            foreach ($doc->items as $node) {
                if (isset($node->request->vc)) continue;

                $cat = new Vod_Category((string)$node->request->fid, (string)$node->title);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        $exist_filters = array();
        if (isset($doc->controls->filters)) {
            foreach ($doc->controls->filters as $filter) {
                $first = reset($filter->items);
                $key = key(array_diff_key((array)$first->request, array('filter' => 'on')));
                $exist_filters[$key] = array('title' => $filter->title, 'values' => array(-1 => TR::t('no')));
                foreach ($filter->items as $item) {
                    $val = $item->request->{$key};
                    $exist_filters[$key]['values'][$val] = $item->title;
                }
            }
            $this->set_filters($exist_filters);
        }

        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Filters count: " . count($exist_filters));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print("getSearchList $keyword");
        $searchRes = $this->make_json_request(array('cmd' => "search", 'query' => $keyword));

        return $searchRes === false ? array() : $this->CollectSearchResult($keyword, $searchRes);
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($params)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $params");

        $pairs = explode(",", $params);
        $post_params = array();
        /** @var array $m */
        foreach ($pairs as $pair) {
            if (preg_match("/^(.+):(.+)$/", $pair, $m)) {
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
            return array();
        }

        $page_idx = $this->get_next_page($params);
        if ($page_idx < 0)
            return array();

        $post_params['filter'] = 'on';
        $post_params['offset'] = $page_idx;
        $json = $this->make_json_request($post_params);

        return $json === false ? array() : $this->CollectSearchResult($params, $json);
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        $page_idx = $this->get_current_page($query_id);
        if ($page_idx < 0) {
            return array();
        }

        $post_params = array('cmd' => "flicks", 'fid' => (int)$query_id, 'offset' => $page_idx, 'limit' => 50);
        $json = $this->make_json_request($post_params);

        return $json === false ? array() : $this->CollectSearchResult($query_id, $json);
    }

    /**
     * @param string $query_id
     * @param $json
     * @return array
     */
    protected function CollectSearchResult($query_id, $json)
    {
        hd_debug_print("query_id: $query_id");
        $movies = array();

        $current_offset = $this->get_current_page($query_id);
        if ($current_offset < 0) {
            return $movies;
        }

        if (!isset($json->items)) {
            hd_debug_print("No items in query! " . json_format_unescaped($json), true);
            return $movies;
        }

        foreach ($json->items as $entry) {
            if ($entry->type === 'next') {
                $this->get_next_page($query_id, $entry->request->offset - $current_offset);
            } else {
                $movie = new Short_Movie(
                    $entry->request->fid,
                    $entry->title,
                    $entry->imglr,
                    TR::t('vod_screen_movie_info__3', $entry->title, $entry->year)
                );
                $movie->big_poster_url = $entry->img;
                $movies[] = $movie;
            }
        }
        if ($current_offset === $this->get_current_page($query_id)) {
            $this->set_next_page($query_id, -1);
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param array|null $params
     * @return false|object
     */
    protected function make_json_request($params = null)
    {
        if (isset($this->embedded_account->vportal)) {
            $vportal = $this->embedded_account->vportal;
        } else {
            $vportal = $this->plugin->get_credentials(Ext_Params::M_VPORTAL);
        }

        /** @var array $m */
        if (empty($vportal)
            || !preg_match('/^portal::\[key:([^]]+)\](.+)$/', $vportal, $m)) {
            hd_debug_print("incorrect or empty VPortal key");
            return false;
        }

        list(, $key, $url) = $m;

        $pairs = array();
        if ($params !== null) {
            $pairs = $params;
        }

        // fill default params
        $pairs['key'] = $key;
        $pairs['mac'] = "000000000000"; // dummy
        $pairs['app'] = "IPTV_ChannelEditor_edem_dune_plugin";

        $curl_opt[CURLOPT_POST] = true;
        $curl_opt[CURLOPT_HTTPHEADER][] = CONTENT_TYPE_JSON;
        $curl_opt[CURLOPT_POSTFIELDS] = $pairs;

        return $this->execApiCommand($url, null, true, $curl_opt);
    }
}

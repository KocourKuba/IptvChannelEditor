<?php
require_once 'lib/default_config.php';

class edem_config extends default_config
{
    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(VOD_SUPPORTED, true);
        $this->set_feature(VOD_QUALITY_SUPPORTED, true);
        $this->set_feature(VOD_FILTER_SUPPORTED, true);
        $this->set_feature(VOD_LAZY_LOAD, true);
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $channel->set_ext_param(M_SUBDOMAIN, isset($this->embedded_account->domain) ? $this->embedded_account->domain : $plugin_cookies->subdomain);
        $channel->set_ext_param(M_TOKEN, isset($this->embedded_account->ott_key) ? $this->embedded_account->ott_key : $plugin_cookies->ott_key);

        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account");

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
        $movieData = $this->make_json_request($plugin_cookies,
            array('cmd' => "flick", 'fid' => (int)$movie_id, 'offset'=> 0,'limit' => 0));

        if ($movieData === false) {
            hd_print("TryLoadMovie: failed to load movie: $movie_id");
            return $movie;
        }

        $series_desc = '';
        if ($movieData->type === 'multistream') {
            // collect series
            //hd_print("movie: $movie_id \"$movieData->title\" contains " . count((array)$movieData->items) . " series");
            foreach ($movieData->items as $item) {
                //hd_print("Try load episode $item->fid playback_url: $item->url");
                $episodeData = $this->make_json_request($plugin_cookies,
                    array('cmd' => "flick", 'fid' => (int)$item->fid, 'offset'=> 0,'limit' => 0));

                if (!isset($episodeData->variants)) {
                    //hd_print("no variants for $item->fid");
                    $movie->add_series_data($item->fid, $item->title, '', $item->url);
                } else if (count((array)$episodeData->variants) === 1) {
                    //hd_print("one variant for $item->fid");
                    $series_desc = "Качество: " . key($episodeData->variants) ."p";
                    $movie->add_series_data($item->fid, $item->title, $series_desc, $item->url);
                } else {
                    $variants_data = (array)$episodeData->variants;
                    //hd_print("episode $item->fid contains " . count($variants_data) . " variants");
                    $variants = array();
                    $series_desc = "Качество: ";
                    foreach ($variants_data as $key => $url) {
                        //hd_print("variant $key playback_url: $url");
                        $quality = ($key === 'auto' ? $key : $key . 'p');
                        $variants[$key] = new Movie_Variant($item->fid . "_" . $key, $quality, $url);
                        if ($key !== 'auto') {
                            $series_desc .= $quality . ", ";
                        }
                    }

                    $series_desc = rtrim($series_desc, " ,\0");
                    $desc = str_replace(array(",", " "), array("|", ""), $series_desc);
                    $movie->add_series_variants_data($item->fid, $item->title, $desc, $variants, $item->url);
                }
            }
        } else if (!isset($movieData->variants)) {
            //hd_print("no variants for $movie_id");
            $movie->add_series_data($movie_id, $movieData->title, '', $movieData->url);
        } else if (count((array)$movieData->variants) === 1) {
            //hd_print("one variant for $movie_id");
            $series_desc = "Качество: " . key($movieData->variants) ."p";
            $movie->add_series_data($movie_id, $movieData->title, $series_desc, $movieData->url);
        } else {
            $variants_data = (array)$movieData->variants;
            //hd_print("movie $movie_id \"$movieData->title\" contains " . count($variants_data) . " variants");
            $variants = array();
            $series_desc = "Качество: ";
            foreach ($variants_data as $key => $url) {
                //hd_print("variant $key playback_url: $url");
                $quality = ($key === 'auto' ? $key : $key . 'p');
                $variants[$key] = new Movie_Variant($movie_id . "_" . $key, $quality, $url);
                if ($key !== 'auto') {
                    $series_desc .= $quality . ", ";
                }
            }

            $series_desc = rtrim($series_desc, " ,\0");
            $desc = str_replace(array(",", " "), array("|", ""), $series_desc);
            $movie->add_series_variants_data($movie_id, $movieData->title, $desc, $variants, $movieData->url);
        }

        $movie->set_data(
            $movieData->title,// caption,
            $series_desc,// caption_original,
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

        $doc = $this->make_json_request($plugin_cookies);
        if ($doc === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        if (isset($doc->items)) {
            foreach ($doc->items as $node) {
                $cat = new Vod_Category((string)$node->request->fid, (string)$node->title);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        $exist_filters = array();
        foreach ($doc->controls->filters as $filter) {
            $first = reset($filter->items);
            $key = key(array_diff_key((array)$first->request, array('filter' => 'on')));
            $exist_filters[$key] = array('title' => $filter->title, 'values' => array(-1 => 'Нет'));
            foreach ($filter->items as $item) {
                $val = $item->request->{$key};
                $exist_filters[$key]['values'][$val] = $item->title;
            }
        }

        $this->set_filters($exist_filters);

        hd_print("Categories read: " . count($category_list));
        hd_print("Filters count: " . count($exist_filters));
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
        $searchRes = $this->make_json_request($plugin_cookies,
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
        $filterRes = $this->make_json_request($plugin_cookies, $post_params);

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
        //hd_print("getVideoList: $query_id, $val");

        $categories = $this->make_json_request($plugin_cookies,
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
     * @param array &$defs
     * @param Starnet_Vod_Filter_Screen $parent
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
     * @param $plugin_cookies
     * @param array|null $params
     * @param bool $to_array
     * @return false|mixed
     */
    protected function make_json_request($plugin_cookies, $params = null, $to_array = false)
    {
        $mediateka = isset($this->embedded_account->vportal) ? $this->embedded_account->vportal : $plugin_cookies->mediateka;
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

        return HD::DownloadJson($url, $to_array, $curl_opt);
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

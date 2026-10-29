<?php
require_once 'lib/default_config.php';

class antifriz_config extends default_config
{
    // movies per page of the VOD lists
    const PAGE_SIZE = 50;

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));
        // this account has special API to get account info
        // {
        //    "data": {
        //        "public_token": "2f5b87bd565ca1234e2sba3fd3919bac",
        //        "private_token": "5acf87d3206a905b83419224703bf666",
        //        "end_time": 1705697968,
        //        "end_date": "2024-01-19 23:59",
        //        "devices_num": 1,
        //        "package": "IPTV HD+SD (позапросный тариф)",
        //        "server": "s01.wsbof.com",
        //        "server_country": "Германия",
        //        "vod": 1,
        //        "ssl": 0,
        //        "disable_adult": 0
        //    }
        // }

        $password = $this->get_password();
        try {
            if (empty($password)) {
                throw new Exception("Password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $params[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
                $params[CURLOPT_CUSTOMREQUEST] = "/auth/info";
                $response = $this->execApiCommand($this->get_feature(Plugin_Constants::PROVIDER_API_URL), null, true, $params);
                if (!isset($response->data)) {
                    throw new Exception("Account info not loaded. " . HD::get_last_error());
                }
                $this->account_data = $response->data;
                $this->plugin->set_credentials(Ext_Params::M_TOKEN, $this->account_data->private_token);
            }
        } catch (Exception $ex) {
            print_backtrace_exception($ex);
            return false;
        }

        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);
        $params[CURLOPT_CUSTOMREQUEST] = "/video/$movie_id";
        $response = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
        if (!isset($response->data)) {
            return null;
        }

        $movie = new Movie($movie_id, $this->plugin);
        $movieData = $response->data;

        $genresArray = array();
        foreach ($movieData->genres as $genre) {
            $genresArray[] = $genre->title;
        }

        $movie->set_data(
            $movieData->name,// caption,
            $movieData->original_name,// caption_original,
            $movieData->description,// description,
            $movieData->poster,// poster_url,
            $movieData->time,// length,
            $movieData->year,// year,
            $movieData->director,// director,
            '',// scenario,
            $movieData->actors,// actors,
            implode(", ", $genresArray),// $xml->genres,
            $movieData->rating,// rate_imdb,
            '',// rate_kinopoisk,
            $movieData->age,// rate_mpaa,
            $movieData->country// country,
        );

        // server and token of the playback url are taken from the account info,
        // it may be not loaded yet if VOD is opened without the TV channels
        $account_data = $this->GetAccountInfo();
        if ($account_data === false || empty($account_data->server)) {
            hd_debug_print("Account info not loaded, can't make movie url");
            return null;
        }

        $domain = $account_data->server;
        $token = $this->plugin->get_credentials(Ext_Params::M_TOKEN);
        $vod_url = (empty($account_data->ssl) ? 'http' : 'https') . '://%s%s?token=%s';
        if (isset($movieData->seasons)) {
            foreach ($movieData->seasons as $season) {
                $movie_season = new Movie_Season($season->number);
                if (!empty($season->name)) {
                    $movie_season->description = $season->name;
                }
                if (!empty($season->original_name)) {
                    $movie_season->description .= empty($season->name) ? $season->original_name : " ($season->original_name)";
                }
                $movie->add_season_data($movie_season);

                foreach ($season->series as $serie) {
                    $playback_url = sprintf($vod_url, $domain, $serie->files[0]->url, $token);
                    $movie_serie = new Movie_Series($serie->id, TR::t('vod_screen_series__1', $serie->number), $playback_url, $season->number);

                    if (!empty($serie->name)) {
                        $movie_serie->description = $serie->name;
                    }
                    if (!empty($movie_serie->original_name)) {
                        $movie_serie->description .= empty($serie->name) ? $serie->original_name : " ($serie->original_name)";
                    }

                    $movie->add_series_data($movie_serie);
                }
            }
        } else {
            $playback_url = sprintf($vod_url, $domain, $movieData->files[0]->url, $token);
            $movie->add_series_data(new Movie_Series($movie_id, $movieData->name, $playback_url));
        }

        return $movie;
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        $jsonItems = $this->execApiCommand($this->GetVodListUrl());
        if ($jsonItems === false) {
            $exception_msg = "Ошибка чтения медиатеки!\n\n" . $this->curl_wrapper->get_raw_response_headers();
            hd_debug_print($exception_msg);
            HD::set_last_error("vod_last_error", $exception_msg);
            return false;
        }

        $category_list = array();
        $category_index = array();

        $total = 0;
        foreach ($jsonItems->data as $node) {
            $id = (string)$node->id;
            $category = new Vod_Category($id, "$node->name ($node->count)");
            $total += $node->count;

            // fetch genres for category
            $params[CURLOPT_CUSTOMREQUEST] = "/cat/$id/genres";
            $genres = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
            if ($genres === false) {
                continue;
            }

            $gen_arr = array();
            if (isset($genres->data)) {
                foreach ($genres->data as $genre) {
                    $gen_arr[] = new Vod_Category((string)$genre->id, (string)$genre->title, $category);
                }
            }

            $category->set_sub_categories($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }

        // all movies
        $category = new Vod_Category(Vod_Category::FLAG_ALL_MOVIES, TR::t('vod_screen_all_movies__1', " ($total)"));
        array_unshift($category_list, $category);
        $category_index[Vod_Category::FLAG_ALL_MOVIES] = $category;

        hd_debug_print("Categories read: " . count($category_list));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print($keyword);
        return $this->get_page_movies($keyword, "/filter/by_name?name=" . urlencode($keyword));
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        hd_debug_print($query_id);

        if ($query_id === Vod_Category::FLAG_ALL_MOVIES) {
            return $this->get_page_movies($query_id, "/filter/new");
        }

        // category_genre
        $arr = explode("_", $query_id);
        $genre_id = isset($arr[1]) ? $arr[1] : $query_id;
        return $this->get_page_movies($query_id, "/genres/$genre_id");
    }

    /**
     * Load next page of the movies list
     *
     * @param string $page_id
     * @param string $request request path without page parameters
     * @return array
     */
    protected function get_page_movies($page_id, $request)
    {
        // page index start from 1
        $page_idx = $this->get_next_page($page_id);
        if ($page_idx < 0)
            return array();

        $params[CURLOPT_CUSTOMREQUEST] = $request . (strpos($request, '?') === false ? '?' : '&') . "page=$page_idx&per_page=" . self::PAGE_SIZE;
        $response = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
        if ($response === false || !isset($response->data)) {
            $this->set_next_page($page_id, -1);
            return array();
        }

        $movies = $this->CollectSearchResult($response);
        if (empty($movies) || $page_idx >= self::get_last_page($response)) {
            // last page
            $this->set_next_page($page_id, -1);
        }

        return $movies;
    }

    /**
     * Last page number of the paginated response (meta.last_page)
     *
     * @param Object $json
     * @return int
     */
    protected static function get_last_page($json)
    {
        return isset($json->meta->last_page) ? (int)$json->meta->last_page : 1;
    }

    /**
     * @param Object $json
     * @return array
     */
    protected function CollectSearchResult($json)
    {
        $movies = array();

        foreach ((array)$json->data as $entry) {
            $genresArray = array();
            if (isset($entry->genres)) {
                foreach ($entry->genres as $genre) {
                    $genresArray[] = $genre->title;
                }
            }
            if (isset($entry->name)) {
                $genre_str = implode(", ", $genresArray);
                $movies[] = new Short_Movie(
                    $entry->id,
                    $entry->name,
                    $entry->poster,
                    TR::t('vod_screen_movie_info__5', $entry->name, $entry->year, $entry->country, $genre_str, $entry->rating)
                );
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }
}

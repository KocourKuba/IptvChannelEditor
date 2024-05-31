<?php
require_once 'lib/default_config.php';

class antifriz_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: $force");
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
                $params[self::API_PARAM_PATH] = "/auth/info";
                $response = $this->execApiCommand($this->get_feature(Plugin_Constants::PROVIDER_API_URL), null, true, $params);
                if (!isset($response->data)) {
                    throw new Exception("Account info not loaded. " . HD::get_last_error());
                }
                $this->account_data = $response->data;
                $this->plugin->set_credentials(Ext_Params::M_TOKEN, $this->account_data->private_token);
            }
        } catch (Exception $ex) {
            hd_debug_print($ex->getMessage());
            return false;
        }

        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print($movie_id);
        $movie = new Movie($movie_id, $this->plugin);

        $params[self::API_PARAM_PATH] = "/video/$movie_id";
        $response = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
        if (!isset($response->data)) {
            return $movie;
        }

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

        $domain = $this->account_data->server;
        $token = $this->plugin->get_credentials(Ext_Params::M_TOKEN);
        $vod_url = 'http://%s%s?token=%s';
        if (isset($movieData->seasons)) {
            foreach ($movieData->seasons as $season) {
                $movie->add_season_data($season->number, !empty($season->name) ? $season->name : TR::t('vod_screen_season__1', $season->number), '');
                foreach ($season->series as $episode) {
                    $name = TR::t('vod_screen_series__2', $episode->number, (empty($episode->name) ? "" : $episode->name));
                    $playback_url = sprintf($vod_url, $domain, $episode->files[0]->url, $token);
                    $movie->add_series_data($episode->id, $name, '', $playback_url, $season->number);
                }
            }
        } else {
            $playback_url = sprintf($vod_url, $domain, $movieData->files[0]->url, $token);
            $movie->add_series_data($movie_id, $movieData->name, '', $playback_url);
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
            $logfile = file_get_contents(get_temp_path(HD::HTTPS_PROXY_LOG));
            $exception_msg = "Ошибка чтения медиатеки!\n\n$logfile";
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
            $params[self::API_PARAM_PATH] = "/cat/$id/genres";
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
        $page_idx = $this->get_next_page($keyword);
        if ($page_idx < 0)
            return array();

        $params[self::API_PARAM_PATH] = "/filter/by_name?name=" . urlencode($keyword) . "&page=$page_idx";
        $response = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
        return $response === false ? array() : $this->CollectSearchResult($response);
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        hd_debug_print($query_id);
        $page_idx = $this->get_next_page($query_id);
        if ($page_idx < 0)
            return array();

        if ($query_id === Vod_Category::FLAG_ALL_MOVIES) {
            $params[self::API_PARAM_PATH] = "/filter/new?page=$page_idx";
        } else {
            $arr = explode("_", $query_id);
            if ($arr === false) {
                $genre_id = $query_id;
            } else {
                $genre_id = $arr[1];
            }

            $params[self::API_PARAM_PATH] = "/genres/$genre_id?page=$page_idx";
        }

        $response = $this->execApiCommand($this->GetVodListUrl(), null, true, $params);
        return $response === false ? array() : $this->CollectSearchResult($response);
    }

    /**
     * @param $json
     * @return array
     */
    protected function CollectSearchResult($json)
    {
        $movies = array();

        foreach ($json->data as $entry) {
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

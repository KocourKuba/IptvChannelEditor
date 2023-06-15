<?php
require_once 'lib/default_config.php';

class cbilling_config extends default_config
{
    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] | string[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print(__METHOD__ . ": Collect information from account: $force");
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

        $password = $this->get_password($plugin_cookies);
        try {
            if (empty($password)) {
                throw new Exception("Password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $headers[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
                $json = HD::DownloadJson($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . '/auth/info', true, $headers);
                if (!isset($json['data'])) {
                    throw new Exception("Account info not loaded");
                }
                $this->account_data = $json['data'];
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return false;
        }

        //foreach($this->account_data as $key => $value) hd_print("  $key => $value");
        return $this->account_data;
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = $this->GetAccountInfo($plugin_cookies, true);
        if ($account_data === false) {
            hd_print(__METHOD__ . ": Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), 20);
            return;
        }

        Control_Factory::add_label($defs, TR::t('packages'), empty($account_data['package']) ? TR::t('no_packages') : $account_data['package'], -10);
        Control_Factory::add_label($defs, TR::t('end_date'), $account_data['end_date'], -10);
        Control_Factory::add_label($defs, TR::t('devices'), $account_data['devices_num'], -10);
        Control_Factory::add_label($defs, TR::t('server_addr'), $account_data['server'], -10);
        Control_Factory::add_label($defs, TR::t('plugin_vod'), ($account_data['vod'] ? TR::t('yes') : TR::t('no')), -10);
        //Control_Factory::add_label($defs, 'Шифрование траффика', ($account_data['ssl'] ? TR::t('yes') : TR::t('no')), 20);
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        hd_print(__METHOD__ . ": $movie_id");
        $movie = new Movie($movie_id, $this->parent);
        $json = HD::DownloadJson($this->GetVodListUrl($plugin_cookies) . "/video/$movie_id", false);
        if ($json === false) {
            return $movie;
        }

        $movieData = $json->data;

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
            $movieData->country,// country,
            ''// budget
        );

        $domain = $this->account_data['server'];
        $token = $this->account_data['private_token'];
        $vod_url = 'http://%s%s?token=%s';
        if (isset($movieData->seasons)) {
            foreach ($movieData->seasons as $season) {
                $movie->add_season_data($season->number, !empty($season->name) ? $season->name : "Сезон $season->number", '');
                foreach ($season->series as $episode) {
                    $name = "Серия $episode->number" . (empty($episode->name) ? "" : " $episode->name");
                    $playback_url = sprintf($vod_url, $domain, $episode->files[0]->url, $token);
                    //hd_print("episode playback_url: $playback_url");
                    $movie->add_series_data($episode->id, $name, '', $playback_url, $season->number);
                }
            }
        } else {
            $playback_url = sprintf($vod_url, $domain, $movieData->files[0]->url, $token);
            //hd_print("movie playback_url: $playback_url");
            $movie->add_series_data($movie_id, $movieData->name, '', $playback_url);
        }

        return $movie;
    }

    /**
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetchVodCategories($plugin_cookies, &$category_list, &$category_index)
    {
        hd_print(__METHOD__);
        $jsonItems = HD::DownloadJson($this->GetVodListUrl($plugin_cookies), false);
        if ($jsonItems === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        $total = 0;
        foreach ($jsonItems->data as $node) {
            $id = (string)$node->id;
            $category = new Vod_Category($id, "$node->name ($node->count)");
            $total += $node->count;

            // fetch genres for category
            $genres = HD::DownloadJson($this->GetVodListUrl($plugin_cookies) . "/cat/$id/genres", false);
            if ($genres === false) {
                continue;
            }

            $gen_arr = array();
            foreach ($genres->data as $genre) {
                $gen_arr[] = new Vod_Category((string)$genre->id, (string)$genre->title, $category);
            }

            $category->set_sub_categories($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }

        // all movies
        $category = new Vod_Category(Vod_Category::FLAG_ALL, "Все фильмы ($total)");
        array_unshift($category_list, $category);
        $category_index[Vod_Category::FLAG_ALL] = $category;

        hd_print(__METHOD__ . ": Categories read: " . count($category_list));
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        //hd_print("__METHOD__");
        $url = $this->GetVodListUrl($plugin_cookies) . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . $this->get_next_page($keyword);
        $searchRes = HD::DownloadJson($url, false);
        return $searchRes === false ? array() : $this->CollectSearchResult($searchRes);
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getMovieList($query_id, $plugin_cookies)
    {
        hd_print(__METHOD__ . ": $query_id");
        $val = $this->get_next_page($query_id);

        if ($query_id === Vod_Category::FLAG_ALL) {
            $url = "/filter/new?page=$val";
        } else {
            $arr = explode("_", $query_id);
            if ($arr === false) {
                $genre_id = $query_id;
            } else {
                $genre_id = $arr[1];
            }

            $url = "/genres/$genre_id?page=$val";
        }

        $categories = HD::DownloadJson($this->GetVodListUrl($plugin_cookies) . $url, false);
        return $categories === false ? array() : $this->CollectSearchResult($categories);
    }

    /**
     * @param $json
     * @return array
     */
    protected function CollectSearchResult($json)
    {
        //hd_print("CollectSearchResult");
        $movies = array();

        foreach ($json->data as $entry) {
            $genresArray = array();
            if (isset($entry->genres)) {
                foreach ($entry->genres as $genre) {
                    $genresArray[] = $genre->title;
                }
            }
            if (isset($entry->name)) {
                $movie = new Short_Movie($entry->id, $entry->name, $entry->poster);
                $genre_str = implode(", ", $genresArray);
                $movie->info = TR::t('vod_screen_movie_info__5', $entry->name, $entry->year, $entry->country, $genre_str, $entry->rating);
                $movies[] = $movie;
            }
        }

        hd_print(__METHOD__ . ": Movies found: " . count($movies));
        return $movies;
    }
}

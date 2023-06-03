<?php
require_once 'lib/default_config.php';

class antifriz_config extends default_config
{
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
        $json = HD::DownloadJson($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/video/$movie_id", false);
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

        $domain = $this->account_data['domain'];
        $token = $this->account_data['token'];
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
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = $this->get_password($plugin_cookies);

        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . '/genres';
    }

    /**
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetchVodCategories($plugin_cookies, &$category_list, &$category_index)
    {
        hd_print("fetch_vod_categories");
        $jsonItems = HD::DownloadJson($this->get_vod_uri($plugin_cookies), false);
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
            $genres = HD::DownloadJson($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/cat/$id/genres", false);
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
        $category = new Vod_Category(Vod_Category::FLAG_ALL, TR::t('vod_screen_all_movies__1', " ($total)"));
        array_unshift($category_list, $category);
        $category_index[Vod_Category::FLAG_ALL] = $category;

        hd_print("Categories read: " . count($category_list));
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        //hd_print("getSearchList");
        $url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL) . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . $this->get_next_page($keyword);
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
        hd_print("getVideoList: $query_id");
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

        $categories = HD::DownloadJson($this->get_feature(Plugin_Constants::PROVIDER_API_URL) . $url, false);
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

        hd_print("Movies found: " . count($movies));
        return $movies;
    }
}

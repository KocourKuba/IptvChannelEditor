<?php
require_once 'default_config.php';

abstract class Cbilling_Vod_Impl extends Default_Config
{
    const API_HOST = 'http://protected-api.com';

    // vod
    const MOVIE_URL_TEMPLATE = 'http://%s%s?token=%s';

    /**
     * @var array
     */
    protected $account_data = array();

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(VOD_LAZY_LOAD, true);
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
        $json = HD::DownloadJson(self::API_HOST . "/video/$movie_id", false);
        if ($json === false) {
            return $movie;
        }
        HD::StoreContentToFile($json, get_temp_path("playlist_vod.json"));

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

        $domain = explode(':', $this->account_data['subdomain']);

        if (isset($movieData->seasons)) {
            foreach ($movieData->seasons as $season) {
                $movie->add_season_data($season->number, !empty($season->name) ? $season->name : "Сезон $season->number", '');
                foreach ($season->series as $episode) {
                    $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $episode->files[0]->url, $this->account_data['token']);
                    //hd_print("episode playback_url: $playback_url");
                    $name = "Серия $episode->number" . (empty($episode->name) ? "" : " $episode->name");
                    $movie->add_series_data($episode->id, $name, '', $playback_url, $season->number);
                }
            }
        } else {
            $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $movieData->files[0]->url, $this->account_data['token']);
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
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        //hd_print("fetch_vod_categories");
        $jsonItems = HD::DownloadJson(self::API_HOST, false);
        if ($jsonItems === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        // all movies
        $category = new Vod_Category(Vod_Category::PATTERN_ALL, 'Все фильмы');
        $category_list[] = $category;
        $category_index[$category->get_id()] = $category;

        foreach ($jsonItems->data as $node) {
            $id = (string)$node->id;
            $category = new Vod_Category($id, (string)$node->name);

            // fetch genres for category
            $genres = HD::DownloadJson(self::API_HOST . "/cat/$id/genres", false);
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
        $url = self::API_HOST . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . $this->get_next_page($keyword);
        $searchRes = HD::DownloadJson($url, false);
        return $searchRes === false ? array() : $this->CollectSearchResult($searchRes);
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        hd_print("getVideoList: $query_id");
        $val = $this->get_next_page($query_id);

        if ($query_id === Vod_Category::PATTERN_ALL) {
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

        $categories = HD::DownloadJson(self::API_HOST . $url, false);
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
                $movie->info = "$entry->name|Год: $entry->year|Страна: $entry->country|Жанр: $genre_str|Рейтинг: $entry->rating";
                $movies[] = $movie;
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
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

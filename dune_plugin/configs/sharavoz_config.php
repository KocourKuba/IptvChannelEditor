<?php
require_once 'lib/default_config.php';
require_once 'lib/xtream/xtream_codes_api.php';

class sharavoz_config extends default_config
{
    /**
     * @var xtream_codes_api
     */
    protected $xtream;

    public function init_defaults()
    {
        parent::init_defaults();

        $this->xtream = new xtream_codes_api();
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        // This is a dummy info taken from xtream codes api
        $this->xtream->init($this->GetVodListUrl(), $this->get_password(), $this->get_password());
        $this->xtream->reset_cache();
        hd_debug_print("Collect information from account: $force");
        // {
        //     "user_info": {
        //     "username": "Yz23423fL234StQ",
        //     "password": "Yz23423fL234StQ",
        //     "message": "OK",
        //     "auth": 1,
        //     "status": "Active",
        //     "exp_date": 1782972755,
        //     "is_trial": "0",
        //     "active_cons": "0",
        //     "created_at": 1703229072,
        //     "max_connections": "1",
        //     "allowed_output_formats": [
        //             "m3u8",
        //             "ts",
        //             "mp4"
        //         ]
        //     },
        //     "server_info": {
        //             "url": "app.media24.cc",
        //         "port": "80",
        //         "https_port": "443",
        //         "server_protocol": "http",
        //         "rtmp_port": "8000",
        //         "timezone": "Europe/Moscow",
        //         "timestamp_now": 1706700536,
        //         "time_now": "2024-01-31 14:28:56"
        //     }
        // }

        $this->account_data = $this->xtream->get_auth();

        return $this->account_data;
    }

    /**
     * @param string $movie_id
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);

        $arr = explode("_", $movie_id);
        $stream_id = isset($arr[0]) ? $arr[0] : $movie_id;
        $stream_type = isset($arr[1]) ? $arr[1] : xtream_codes_api::VOD;

        $item = $this->xtream->get_stream_info($stream_id, $stream_type);

        if ($item === false) {
            hd_debug_print("failed to load movie: $stream_id from: $stream_type");
            return null;
        }

        // VOD response
        //
        // {
        //    "info": {
        //        "kinopoisk_url": "https://www.kinopoisk.ru/film/1130339/",
        //        "name": "Дивизион",
        //        "o_name": "Дивизион",
        //        "cover_big": "https://avatars.mds.yandex.net/get-kinopoisk-image/1704946/adf2412f-0f8d-4090-9381-dd96b5d0fcfb/x1000",
        //        "movie_image": "https://avatars.mds.yandex.net/get-kinopoisk-image/1704946/adf2412f-0f8d-4090-9381-dd96b5d0fcfb/x1000",
        //        "releasedate": 2019,
        //        "episode_run_time": 0,
        //        "youtube_trailer": "",
        //        "director": "Родриго Монте, Висенте Аморин",
        //        "actors": "Эром Кордейру, Силвиу Гиндани
        //        "cast": "",
        //        "description": "В 1997 году Рио-де-Жанейро потрясла волна похищений людей.
        //        "plot": "В 1997 году Рио-де-Жанейро потрясла волна похищений людей.
        //        "age": "16+",
        //        "rating_mpaa": "",
        //        "rating_count_kinopoisk": "48",
        //        "country": "Бразилия",
        //        "genre": "боевик, триллер, драма, криминал",
        //        "backdrop_path": [],
        //        "tmdb_id": "476299",
        //        "rating": "7.9",
        //        "duration_secs": 0,
        //        "duration": null,
        //        "container_extension": "mp4"
        //    },
        // }

        // Serials response
        //
        //{
        //    "info": {
        //        "name": "Уэнсдэй",
        //        "cover": "http://static.media24.cc/static/posters/304d86b6aeab.jpeg",
        //        "releaseDate": "2022",
        //        "episode_run_time": 45,
        //        "youtube_trailer": null,
        //        "director": "Тим Бёртон, Джеймс Маршалл, Ганджа Монтейру",
        //        "cast": "Дженна Ортега
        //        "plot": "Уэнсдэй, дочь Гомеса и Мортиши Аддамс, учится в академии Nevermore.
        //        "last_modified": 1705606092,
        //        "genre": "фэнтези, комедия, криминал, детектив",
        //        "category_id": 3,
        //        "backdrop_path": null
        //    },
        //    "episodes": {
        //        "1": [
        //            {
        //                "id": 26032,
        //                "episode_num": 1,
        //                "series_id": 3,
        //                "title": "Дочь среды - сестра беды",
        //                "container_extension": "mp4",
        //                "movie_image": "http://static.media24.cc/static/posters/26032.jpeg",
        //                "info": {},
        //                "custom_sid": "",
        //                "added": 1656766454,
        //                "season": 1,
        //                "direct_source": ""
        //            },
        //    },
        //

        $movie = new Movie($movie_id, $this->plugin);
        if ($stream_type === xtream_codes_api::VOD) {
            $movie->set_data(
                $item->info->name,                    // name,
                $item->info->o_name,                  // name_original,
                $item->info->plot,                    // description,
                $item->info->movie_image,             // poster_url,
                $item->info->duration,                // length_min,
                $item->info->releasedate,             // year,
                $item->info->director,                // director_str,
                '',                       // scenario_str,
                $item->info->actors,                  // actors_str,
                $item->info->genre,                   // genres_str,
                $item->info->rating,                  // rate_imdb,
                $item->info->rating_count_kinopoisk,  // rate_kinopoisk,
                $item->info->age,                     // rate_mpaa,
                $item->info->country,                 // country,
                ''                             // budget
            );

            if (empty($item->info->container_extension)) {
                $id = $stream_id;
            } else {
                $id = "$stream_id.{$item->info->container_extension}";
            }
            $url = $this->xtream->get_stream_url($id);
            hd_debug_print("movie playback_url: $url");
            $movie->add_series_data($movie_id, $item->info->name, '', $url);
        }

        if ($stream_type === xtream_codes_api::SERIES) {
            $movie->set_data(
                $item->info->name,                    // name,
                '',                       // name_original,
                $item->info->plot,                    // description,
                $item->info->cover,                   // poster_url,
                $item->info->episode_run_time,        // length_min,
                $item->info->releaseDate,             // year,
                $item->info->director,                // director_str,
                '',                       // scenario_str,
                $item->info->cast,                    // actors_str,
                $item->info->genre,                   // genres_str,
                '',                         // rate_imdb,
                '',                      // rate_kinopoisk,
//                $item->info->rating,                  // rate_imdb,
//                $item->info->rating_count_kinopoisk,  // rate_kinopoisk,
                '',                         // rate_mpaa,
                '',                           // country,
                ''                             // budget
            );

            foreach ($item->episodes as $season_id => $season) {
                $movie->add_season_data($season_id, !empty($season->name) ? $season->name : TR::t('vod_screen_season__1', $season_id), '');
                foreach ($season as $episode) {
                    $name = TR::t('vod_screen_series__2', $episode->episode_num, (empty($episode->title) ? "" : $episode->title));
                    if (empty($episode->container_extension)) {
                        $id = $episode->id;
                    } else {
                        $id = "$episode->id.$episode->container_extension";
                    }
                    $url = $this->xtream->get_stream_url($id);
                    hd_debug_print("episode playback_url: $url");
                    $movie->add_series_data($episode->id, $name, '', $url, $season_id, $episode->movie_image);
                }
            }
        }

        return $movie;
    }

    /**
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        hd_debug_print(null, true);

        $category_tree = array();
        $this->parse_categories(xtream_codes_api::VOD, $category_tree);
        $this->parse_categories(xtream_codes_api::SERIES, $category_tree);

        $category_count = 0;
        foreach ($category_tree as $key => $value) {
            $category = new Vod_Category($key, $key);
            $gen_arr = array();
            foreach ($value as $sub_cat_name) {
                $sub_pair = explode("_", $sub_cat_name);
                hd_debug_print("Sub Category ($sub_pair[2]): $sub_pair[0] ($sub_pair[1])", true);
                $gen_arr[] = new Vod_Category($sub_pair[1] . "_" . $sub_pair[2], $sub_pair[0], $category);
            }

            $category->set_sub_categories($gen_arr);
            $category_count += count($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }
        hd_debug_print("Categories read: $category_count");
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $query_id
     * @return array
     * @throws Exception
     */
    public function getMovieList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($query_id);

        // Фильмы_1_vod
        $arr = explode("_", $query_id);
        $category_id = isset($arr[1]) ? $arr[1] : $query_id;

        $vod_items = $this->xtream->get_streams($arr[2], $category_id);
        $current_offset = $this->get_next_page($query_id, 0);

        $pos = 0;
        $movies = array();
        foreach ($vod_items as $movie) {
            if ($pos++ < $current_offset) continue;

            $category = (string)$movie->category_id;
            if (empty($category)) {
                $category = TR::load_string('no_category');
            }

            if ($category_id === Vod_Category::FLAG_ALL || $category_id === $category) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }
        $this->get_next_page($query_id, $pos - $current_offset);

        hd_debug_print("Movies read for query: $query_id: " . count($movies));
        return $movies;
    }

    /**
     * @param string $keyword
     * @return array
     * @throws Exception
     */
    public function getSearchList($keyword)
    {
        hd_debug_print(null, true);
        hd_debug_print($keyword);

        $movies = array();

        $this->search(xtream_codes_api::VOD, $keyword, $movies);
        $this->search(xtream_codes_api::SERIES, $keyword, $movies);

        hd_debug_print("Movies found: " . count($movies));

        return array_values($movies);
    }

    /**
     * @param Object $movie_obj
     * @return Short_Movie
     */
    protected static function CreateShortMovie($movie_obj)
    {
        $id = '-1';
        $icon = '';
        if (isset($movie_obj->stream_id)) {
            $id = $movie_obj->stream_id . "_" . xtream_codes_api::VOD;
            $icon = (string)$movie_obj->stream_icon;
        } else if (isset($movie_obj->series_id)) {
            $id = $movie_obj->series_id . "_" . xtream_codes_api::SERIES;
            $icon = (string)$movie_obj->cover;
        }

        return new Short_Movie(
            $id,
            (string)$movie_obj->name,
            $icon,
            TR::t('vod_screen_movie_info__2', $movie_obj->name, $movie_obj->rating)
        );
    }

    /**
     * @param string $stream_type
     * @param array &$category_tree
     */
    protected function parse_categories($stream_type, &$category_tree)
    {
        $categories = $this->xtream->get_categories($stream_type);
        if ($categories !== false) {
            foreach ($categories as $item) {
                hd_debug_print("$item->category_id ($item->category_name)", true);
                $pair = explode("|", $item->category_name);

                $parent_id = trim($pair[0]);
                $query_id = trim($pair[1]) . "_" . $item->category_id . "_" . $stream_type;
                $category_tree[$parent_id][] = $query_id;
            }
        }
    }

    protected function search($stream_type, $keyword, &$movies)
    {
        $categories = $this->xtream->get_categories($stream_type);
        if ($categories === false) {
            return;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($categories as $category) {
            $streams = $this->xtream->get_streams($stream_type, $category->category_id);
            if ($streams === false) continue;

            foreach ($streams as $stream) {
                $search = utf8_encode(mb_strtolower($stream->name, 'UTF-8'));
                if (strpos($search, $keyword) !== false) {
                    $id = $stream_type === xtream_codes_api::SERIES ? $stream->series_id : $stream->stream_id;
                    $movies[$id] = self::CreateShortMovie($stream);
                }
            }
        }
    }
}

<?php
require_once 'lib/default_config.php';

class glanz_config extends default_config
{
    public function init_defaults($short_name)
    {
        parent::init_defaults($short_name);

        $this->set_feature(VOD_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(VOD_FILTER_SUPPORTED, true);
        $this->set_feature(VOD_PLAYLIST_URL, 'http://api.ottg.tv/playlist/vod?login=%s&password=%s');
    }

    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS,true);
        $this->set_feature(ACCESS_TYPE,ACCOUNT_LOGIN);
        $this->set_feature(PLAYLIST_TEMPLATE,'http://pl.ottg.cc/get.php?username={LOGIN}&password={PASSWORD}&type=m3u&output=hls');
        $this->set_feature(URI_PARSE_PATTERN,'^https?://(?<domain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$');

        $this->set_stream_param(HLS,CU_TYPE,'flussonic');
        $this->set_stream_param(HLS,CU_SUBST,'video');
        $this->set_stream_param(HLS,URL_TEMPLATE,'http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE,'http://{DOMAIN}/{ID}/video-{CU_SUBST}-{DURATION}.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');

        $this->set_stream_param(MPEG,URL_TEMPLATE,'http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE,'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.iptvx.one/api/id/{ID}.json');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT,'ch_programme');
        $this->set_epg_param(EPG_FIRST,EPG_START,'start');
        $this->set_epg_param(EPG_FIRST,EPG_END,'');
        $this->set_epg_param(EPG_FIRST,EPG_NAME,'title');
        $this->set_epg_param(EPG_FIRST,EPG_DESC,'description');
        $this->set_epg_param(EPG_FIRST,EPG_TIME_FORMAT,'{DAY}-{MONTH}-{YEAR} {HOUR}:{MIN}'); // 'd-m-Y H:i'
        $this->set_epg_param(EPG_FIRST,EPG_TIMEZONE,3); // // iptvx.one uses moscow time (UTC+3)

        $this->set_epg_param(EPG_SECOND,EPG_URL,'http://epg.drm-play.ml/iptvx.one/epg/{ID}.json');
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        return sprintf($this->get_feature(VOD_PLAYLIST_URL), $login, $password);
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
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());

        if ($jsonItems === false) {
            hd_print("TryLoadMovie: failed to load movie: $movie_id");
            return $movie;
        }

        foreach ($jsonItems as $item) {
            if (isset($item->id)) {
                $id = (string)$item->id;
            } else {
                $id = '-1';
            }

            if ($id !== $movie_id) {
                continue;
            }

            $genres = array();
            foreach ($item->genres as $genre) {
                if (!empty($genre->title)) {
                    $genres[] = $genre->title;
                }
            }
            $genres_str = implode(", ", $genres);

            $movie->set_data(
                $item->name,            // name,
                $item->o_name,          // name_original,
                $item->description,     // description,
                $item->cover,           // poster_url,
                '',           // length_min,
                $item->year,            // year,
                $item->director,        // director_str,
                '',           // scenario_str,
                $item->actors,          // actors_str,
                $genres_str,            // genres_str,
                '',            // rate_imdb,
                '',         // rate_kinopoisk,
                '',            // rate_mpaa,
                $item->country,         // country,
                ''               // budget
            );

            $playback_url = str_replace("https://", "http://", $item->url);
            hd_print("movie playback_url: $playback_url");
            $movie->add_series_data($movie_id, $item->name, '', $playback_url);
            break;
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
        $url = $this->GetVodListUrl($plugin_cookies);
        $jsonItems = HD::DownloadJson($url, false);
        if ($jsonItems === false) {
            return;
        }

        HD::StoreContentToFile($jsonItems, self::get_vod_cache_file());

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        // all movies
        $category = new Vod_Category(Vod_Category::PATTERN_ALL, 'Все фильмы');
        $category_list[] = $category;
        $category_index[$category->get_id()] = $category;

        $genres = array();
        $years = array();
        foreach ($jsonItems as $movie) {
            $category = (string)$movie->category;
            if (empty($category)) {
                $category = "Без категории";
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }

            // collect filters information
            $year = (int)$movie->year;
            $years[$year] = $movie->year;
            foreach ($movie->genres as $genre) {
                $val = (int)$genre->id;
                $genres[$val] = $genre->title;
            }
        }

        ksort($genres);
        krsort($years);

        $filters = array();
        $filters['genre'] = array('title' => 'Жанр', 'values' => array(-1 => 'Нет'));
        $filters['from'] = array('title' => 'Год от', 'values' => array(-1 => 'Нет'));
        $filters['to'] = array('title' => 'Год до', 'values' => array(-1 => 'Нет'));

        $filters['genre']['values'] += $genres;
        $filters['from']['values'] += $years;
        $filters['to']['values'] += $years;

        $this->set_filters($filters);

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
        hd_print("getSearchList: $keyword");
        $movies = array();
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print("getSearchList: failed to load movies");
            return $movies;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item->name, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateShortMovie($item);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();

        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print("getVideoList: failed to load movies");
            return $movies;
        }

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        foreach ($jsonItems as $movie) {
            $category = $movie->category;
            if (empty($category)) {
                $category = "Без категории";
            }

            if ($category_id === Vod_Category::PATTERN_ALL || $category_id === $category) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }

        hd_print("Movies read for query: $query_id - " . count($movies));
        return $movies;
    }

    /**
     * @param string $params
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getFilterList($params, $plugin_cookies)
    {
        hd_print("getFilterList: $params");
        $movies = array();

        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print("getFilterList: failed to load movies");
            return $movies;
        }

        $pairs = explode(",", $params);
        $post_params = array();
        foreach ($pairs as $pair) {
            if (preg_match("|^(.+):(.+)$|", $pair, $m)) {
                $filter = $this->get_filter($m[1]);
                if ($filter !== null && !empty($filter['values'])) {
                    $item_idx = array_search($m[2], $filter['values']);
                    if ($item_idx !== false && $item_idx !== -1) {
                        $post_params[$m[1]] = (int)$item_idx;
                    }
                }
            }
        }

        foreach ($jsonItems as $movie) {
            $match_genre = !isset($post_params['genre']);
            if (!$match_genre) {
                foreach ($movie->genres as $genre) {
                    if (!isset($post_params['genre']) || (int)$genre->id === $post_params['genre']) {
                        $match_genre = true;
                        break;
                    }
                }
            }

            $match_year = false;
            $year_from = isset($post_params['from']) ? $post_params['from'] : ~PHP_INT_MAX;
            $year_to = isset($post_params['to']) ? $post_params['to'] : PHP_INT_MAX;

            if ((int)$movie->year >= $year_from && (int)$movie->year <= $year_to) {
                $match_year = true;
            }

            if ($match_year && $match_genre) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param Object $movie_obj
     * @return Short_Movie
     */
    protected static function CreateShortMovie($movie_obj)
    {
        if (isset($movie_obj->id)) {
            $id = (string)$movie_obj->id;
        } else {
            $id = '-1';
        }

        $genres = array();
        foreach ($movie_obj->genres as $genre) {
            if (!empty($genre->title)) {
                $genres[] = $genre->title;
            }
        }
        $genres_str = implode(", ", $genres);

        $movie = new Short_Movie($id, (string)$movie_obj->name, (string)$movie_obj->cover);
        $movie->info = "$movie_obj->name|Год: $movie_obj->year|Страна: $movie_obj->country|Жанр: $genres_str";

        return $movie;
    }

    /**
     * @param array &$defs
     * @param Starnet_Vod_Filter_Screen $parent
     * @param int $initial
     * @return bool
     */
    public function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        $filters = array("genre", "from", "to");
        hd_print("AddFilterUI: $initial");
        Control_Factory::add_vgap($defs, 20);
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
        $filters = array("genre", "from", "to");
        $compiled_string = "";
        foreach ($filters as $name) {
            $filter = $this->get_filter($name);
            if ($filter !== null && $user_input->{$name} !== -1) {
                if (!empty($compiled_string)) {
                    $compiled_string .= ",";
                }

                $compiled_string .= $name . ":" . $filter['values'][$user_input->{$name}];
            }
        }

        return $compiled_string;
    }

    protected static function get_vod_cache_file()
    {
        return get_temp_path("playlist_vod.json");
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

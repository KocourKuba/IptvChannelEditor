<?php
require_once 'lib/default_config.php';

class glanz_config extends default_config
{
    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(Plugin_Constants::VOD_FILTER_SUPPORTED, true);
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
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());

        if ($jsonItems === false) {
            hd_print(__METHOD__ . ": failed to load movie: $movie_id");
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
                '',         // scenario_str,
                $item->actors,          // actors_str,
                $genres_str,            // genres_str,
                '',           // rate_imdb,
                '',        // rate_kinopoisk,
                '',           // rate_mpaa,
                $item->country,         // country,
                ''               // budget
            );

            hd_print(__METHOD__ . ": movie playback_url: $item->url");
            $movie->add_series_data($movie_id, $item->name, '', $item->url);
            break;
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
        $jsonItems = HD::DownloadJson($this->GetVodListUrl($plugin_cookies), false);
        if ($jsonItems === false) {
            return;
        }

        HD::StoreContentToFile($jsonItems, self::get_vod_cache_file());

        $category_list = array();
        $category_index = array();
        $cat_info = array();

        // all movies
        $count = count($jsonItems);
        $cat_info[Vod_Category::FLAG_ALL] = $count;
        $genres = array();
        $years = array();
        foreach ($jsonItems as $movie) {
            $category = (string)$movie->category;
            if (empty($category)) {
                $category = TR::t('no_category');
            }

            if (!array_key_exists($category, $cat_info)) {
                $cat_info[$category] = 0;
            }

            ++$cat_info[$category];

            // collect filters information
            $years[(int)$movie->year] = $movie->year;
            foreach ($movie->genres as $genre) {
                $genres[(int)$genre->id] = $genre->title;
            }
        }

        foreach ($cat_info as $category => $movie_count) {
            $cat = new Vod_Category($category,
                ($category === Vod_Category::FLAG_ALL) ? TR::t('vod_screen_all_movies__1', "($movie_count)") : "$category ($movie_count)");
            $category_list[] = $cat;
            $category_index[$category] = $cat;
        }

        ksort($genres);
        krsort($years);

        $filters = array();
        $filters['genre'] = array('title' => TR::t('genre'), 'values' => array(-1 => TR::t('no')));
        $filters['from'] = array('title' => TR::t('year_from'), 'values' => array(-1 => TR::t('no')));
        $filters['to'] = array('title' => TR::t('year_to'), 'values' => array(-1 => TR::t('no')));

        $filters['genre']['values'] += $genres;
        $filters['from']['values'] += $years;
        $filters['to']['values'] += $years;

        $this->set_filters($filters);

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
        hd_print(__METHOD__ . ": $keyword");
        $movies = array();
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print(__METHOD__ . ": failed to load movies");
            return $movies;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item->name, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateShortMovie($item);
            }
        }

        hd_print(__METHOD__ . ": Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getMovieList($query_id, $plugin_cookies)
    {
        $movies = array();

        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print(__METHOD__ . ": failed to load movies");
            return $movies;
        }

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        $current_offset = $this->get_next_page($query_id, 0);
        $pos = 0;
        foreach ($jsonItems as $movie) {
            if ($pos++ < $current_offset) continue;

            $category = $movie->category;
            if (empty($category)) {
                $category = TR::t('no_category');
            }

            if ($category_id === Vod_Category::FLAG_ALL || $category_id === $category) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }
        $this->get_next_page($query_id, $pos - $current_offset);

        hd_print(__METHOD__ . ": Movies read for query: $query_id - " . count($movies));
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
        hd_print(__METHOD__ . ": $params");
        $movies = array();

        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print(__METHOD__ . ": failed to load movies");
            return $movies;
        }

        $pairs = explode(",", $params);
        $post_params = array();
        foreach ($pairs as $pair) {
            if (preg_match("/^(.+):(.+)$/", $pair, $m)) {
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

        hd_print(__METHOD__ . ": Movies found: " . count($movies));
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
        $movie->info = TR::t('vod_screen_movie_info__4', $movie_obj->name, $movie_obj->year, $movie_obj->country, $genres_str);

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
        hd_print(__METHOD__ . ": $initial");
        Control_Factory::add_vgap($defs, 20);
        $added = false;
        foreach ($filters as $name) {
            $filter = $this->get_filter($name);
            if ($filter === null) {
                hd_print(__METHOD__ . ": no filters with '$name'");
                continue;
            }

            $values = $filter['values'];
            if (empty($values)) {
                hd_print(__METHOD__ . ": no filters values for '$name'");
                continue;
            }

            $idx = $initial;
            if ($initial !== -1) {
                $pairs = explode(" ", $initial);
                foreach ($pairs as $pair) {
                    if (strpos($pair, $name . ":") !== false && preg_match("/^$name:(.+)/", $pair, $m)) {
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
}

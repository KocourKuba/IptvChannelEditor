<?php
require_once 'lib/default_config.php';

class glanz_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);

        $movie = new Movie($movie_id, $this->plugin);
        if ($this->vod_items === false) {
            hd_debug_print("failed to load movie: $movie_id");
            return $movie;
        }

        foreach ($this->vod_items as $item) {
            $item = (object)$item;
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
                $item->country          // country,
            );

            hd_debug_print("movie playback_url: $item->url");
            $movie->add_series_data(new Movie_Series($movie_id, $item->name, $item->url));
            break;
        }

        return $movie;
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        $this->perf->reset('start');
        if ($this->load_vod_json_full(true) === false) {
            return false;
        }

        $count = count($this->vod_items);
        $category_list = array();
        $category_index = array();
        $cat_info = array();

        // all movies
        $cat_info[Vod_Category::FLAG_ALL_MOVIES] = $count;
        $genres = array();
        $countries = array();
        $years = array();
        foreach ($this->vod_items as $movie) {
            $category = safe_get_value($movie, 'category');
            if (empty($category)) {
                $category = TR::load('no_category');
            }

            if (!array_key_exists($category, $cat_info)) {
                $cat_info[$category] = 0;
            }

            ++$cat_info[$category];

            // collect filters information
            $year = (int)safe_get_value($movie, 'year');
            $years[$year] = $year;

            foreach (explode(',', safe_get_value($movie, 'country')) as $item) {
                $item = trim($item);
                if (!empty($item)) {
                    $countries[$item] = $item;
                }
            }

            foreach (safe_get_value($movie, 'genres', array()) as $genre) {
                $id = (int)safe_get_value($genre, 'id');
                $title = safe_get_value($genre, 'title');
                if (!empty($title) && !empty($id)) {
                    $genres[$id] = $title;
                }
            }
        }

        foreach ($cat_info as $category => $movie_count) {
            $cat = new Vod_Category($category,
                ($category === Vod_Category::FLAG_ALL_MOVIES) ? TR::t('vod_screen_all_movies__1', "($movie_count)") : "$category ($movie_count)");
            $category_list[] = $cat;
            $category_index[$category] = $cat;
        }

        ksort($genres);
        ksort($countries);
        krsort($years);

        $filters = array();
        $filters['genre'] = array('title' => TR::t('genre'), 'values' => array(-1 => TR::t('no')));
        $filters['country'] = array('title' => TR::t('country'), 'values' => array(-1 => TR::t('no')));
        $filters['from'] = array('title' => TR::t('year_from'), 'values' => array(-1 => TR::t('no')));
        $filters['to'] = array('title' => TR::t('year_to'), 'values' => array(-1 => TR::t('no')));

        $filters['genre']['values'] += $genres;
        $filters['country']['values'] += $countries;
        $filters['from']['values'] += $years;
        $filters['to']['values'] += $years;

        $this->set_filter_types($filters);

        $this->perf->setLabel('end');
        $report = $this->perf->getFullReport();

        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Total items loaded: " . count($this->vod_items));
        hd_debug_print("Load time: {$report[Perf_Collector::TIME]} sec");
        hd_debug_print("Memory usage: {$report[Perf_Collector::MEMORY_USAGE_KB]} kb");
        hd_debug_print_separator();

        return true;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print($keyword);
        $movies = array();
        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return $movies;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($this->vod_items as $movieData) {
            $search = utf8_encode(mb_strtolower(safe_get_value($movieData, 'name'), 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = $this->CreateShortMovie($movieData);
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        $movies = array();

        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return $movies;
        }

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        $page_idx = $this->get_current_page($query_id);
        if ($page_idx < 0)
            return array();

        $pos = 0;
        foreach ($this->vod_items as $movieData) {
            if ($pos++ < $page_idx) continue;

            $category = safe_get_value($movieData, 'category');
            if (empty($category)) {
                $category = TR::load('no_category');
            }

            if ($category_id === Vod_Category::FLAG_ALL_MOVIES || $category_id === $category) {
                $movies[] = $this->CreateShortMovie($movieData);
            }
        }
        $this->get_next_page($query_id, $pos - $page_idx);

        hd_debug_print("Movies read for query: $query_id - " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $query_id");

        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return array();
        }

        $filter_params = $this->get_filter_params($query_id);

        $movies = array();

        foreach ($this->vod_items as $movieData) {
            $match_genre = true;
            $match_country = true;
            $match_year = true;

            if (isset($filter_params['genre'])) {
                $genres = array_map(function($item) {
                    return safe_get_value($item, 'id');
                }, safe_get_value($movieData, 'genres', array()));
                $match_genre = !empty($genres) && in_array($filter_params['genre'], $genres);
            }

            if (isset($filter_params['country'])) {
                $country_str = safe_get_value($movieData, 'country');
                $match_country = strpos($country_str, $filter_params['country']) !== false;
            }

            if (isset($filter_params['from']) || isset($filter_params['to'])) {
                $match_year = false;
                $year_from = safe_get_value($filter_params, 'from', ~PHP_INT_MAX);
                $year_to = safe_get_value($filter_params, 'to', PHP_INT_MAX);
                $year = (int)safe_get_value($movieData, 'year');
                if ($year >= $year_from && $year <= $year_to) {
                    $match_year = true;
                }
            }

            if ($match_year && $match_genre && $match_country) {
                $movies[] = $this->CreateShortMovie($movieData);
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param array $movieData
     * @return Short_Movie
     */
    protected function CreateShortMovie($movieData)
    {
        $id = (string)safe_get_value($movieData, 'id');
        if (empty($id)) {
            $id = '-1';
        }

        $genres = array();
        foreach (safe_get_value($movieData, 'genres', array()) as $genre) {
            $title = safe_get_value($genre, 'title');
            if (!empty($title)) {
                $genres[] = $title;
            }
        }
        $genres_str = implode(", ", $genres);

        $name = safe_get_value($movieData, 'name');
        return new Short_Movie(
            $id,
            safe_get_value($movieData, 'name'),
            safe_get_value($movieData, 'cover'),
            TR::t('vod_screen_movie_info__4',
                $name,
                safe_get_value($movieData, 'year'),
                safe_get_value($movieData, 'country'),
                $genres_str)
        );
    }
}

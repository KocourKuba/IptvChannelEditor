<?php
require_once 'lib/default_config.php';

class ipstream_config extends default_config
{
    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);
        $jsonItems = parse_json_file(self::get_vod_cache_file(), false);

        if ($jsonItems === false) {
            hd_debug_print("failed to load movie: $movie_id");
            return null;
        }

        $movie = null;
        foreach ($jsonItems as $item) {
            if (isset($item->id)) {
                $id = (string)$item->id;
            } else if (isset($item->series_id)) {
                $id = $item->series_id . "_serial";
            } else {
                $id = Hashed_Array::hash($item->name);
            }

            if ($id !== $movie_id) {
                continue;
            }

            $duration = "";
            if (isset($item->info->duration_secs)) {
                $duration = (int)$item->info->duration_secs / 60;
            } else if (isset($item->info->episode_run_time)) {
                $duration = (int)$item->info->episode_run_time;
            }

            $age = isset($item->info->adult) && !empty($item->info->adult) ? "{$item->info->adult}+" : '';
            $age_limit = empty($age) ? array() : array(TR::t('vod_screen_age_limit') => $age);

            $movie = new Movie($movie_id, $this->plugin);
            $movie->set_data(
                $item->name,                          // name,
                '',                       // name_original,
                $item->info->plot,                    // description,
                $item->info->poster,                  // poster_url,
                $duration,                            // length_min,
                $item->info->year,                    // year,
                HD::ArrayToStr($item->info->director),// director_str,
                '',                       // scenario_str,
                HD::ArrayToStr($item->info->cast),    // actors_str,
                HD::ArrayToStr($item->info->genre),   // genres_str,
                $item->info->rating,                  // rate_imdb,
                '',                      // rate_kinopoisk,
                '',                         // rate_mpaa,
                HD::ArrayToStr($item->info->country),  // country,
                '',
                array(), // details
                $age_limit // rate details
            );

            // case for serials
            if (isset($item->seasons)) {
                foreach ($item->seasons as $season) {
                    $movie_season = new Movie_Season($season->season);

                    if (!empty($season->info->overview)) {
                        $movie_season->description = $season->info->overview;
                    }

                    if (!empty($season->info->air_date)) {
                        $title = empty($movie_season->description) ? $movie_season->name : $movie_season->description;
                        $movie_season->description = TR::t('vod_screen_air_date__2', $title, $season->info->air_date);
                    }

                    if (!empty($season->info->poster)) {
                        $movie_season->poster = $season->info->poster;
                    }

                    $movie->add_season_data($movie_season);

                    foreach ($season->episodes as $episode) {
                        $movie_serie = new Movie_Series("$movie_season->name:$episode->episode",
                            TR::t('vod_screen_series__1', $episode->episode),
                            $episode->video,
                            $season->season);
                        $movie->add_series_data($movie_serie);
                    }
                }
            } else {
                hd_debug_print("movie playback_url: $item->video");
                $movie->add_series_data(new Movie_Series($movie_id, $item->name, $item->video));
            }

            break;
        }

        return $movie;
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        if ($this->load_vod_json_full(true) === false) {
            return false;
        }

        $category_list = array();
        $category_index = array();
        $cat_info = array();

        // all movies
        $count = count($this->vod_items);
        $cat_info[Vod_Category::FLAG_ALL_MOVIES] = $count;
        $genres = array();
        $countries = array();
        $years = array();
        foreach ($this->vod_items as $movie) {
            $movie = (object)$movie;
            $category = (string)$movie->category;
            if (empty($category)) {
                $category = TR::load('no_category');
            }

            if (!array_key_exists($category, $cat_info)) {
                $cat_info[$category] = 0;
            }

            ++$cat_info[$category];

            // collect filters information
            $movie_year = (int)$movie->info['year'];
            $years[$movie_year] = $movie_year;
            foreach ($movie->info['genre'] as $genre) {
                if (!empty($genre)) {
                    $genres[$genre] = $genre;
                }
            }

            foreach ($movie->info['country'] as $country) {
                if (!empty($country)) {
                    $countries[$country] = $country;
                }
            }
        }

        foreach ($cat_info as $category => $movie_count) {
            $cat = new Vod_Category($category,
                ($category === Vod_Category::FLAG_ALL_MOVIES) ? TR::t('vod_screen_all_movies__1', " ($movie_count)") : "$category ($movie_count)");
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

        hd_debug_print("Categories read: " . count($category_list));
        return true;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print($keyword);
        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return array();
        }

        $movies = array();
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
        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return array();
        }

        $page_idx = $this->get_current_page($query_id);
        if ($page_idx < 0)
            return array();

        $movies = array();
        $arr = explode("_", $query_id);
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

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
                $genres = safe_get_value($movieData, array('info', 'genre'), array());
                $match_genre = !empty($genres) && in_array($filter_params['genre'], $genres);
            }

            if (isset($filter_params['country'])) {
                $countries = safe_get_value($movieData, array('info', 'country'), array());
                $match_country = !empty($countries) && in_array($filter_params['country'], $countries);
            }

            if (isset($filter_params['from']) || isset($filter_params['to'])) {
                $match_year = false;
                $year_from = safe_get_value($filter_params, 'from', ~PHP_INT_MAX);
                $year_to = safe_get_value($filter_params, 'to', PHP_INT_MAX);
                $year = (int)safe_get_value($movieData, array('info', 'year'));
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
        $name = safe_get_value($movieData, 'name');
        if (isset($movieData['id'])) {
            $id = (string)$movieData['id'];
        } else if (isset($movieData['series_id'])) {
            $id = $movieData['series_id'] . "_serial";
        } else {
            $id = Hashed_Array::hash($name);
        }

        $movie_info = safe_get_value($movieData, 'info', array());
        $genres = HD::ArrayToStr(safe_get_value($movie_info, 'genre'));
        $country = HD::ArrayToStr(safe_get_value($movie_info, 'country'));

        $movie = new Short_Movie(
            $id,
            safe_get_value($movieData, 'name'),
            safe_get_value($movie_info, 'poster'),
            TR::t('vod_screen_movie_info__5',
                $name,
                safe_get_value($movie_info, 'year'),
                $country,
                $genres,
                safe_get_value($movie_info, 'rating')
            )
        );

        $this->plugin->vod->set_cached_short_movie($movie);

        return $movie;
    }
}

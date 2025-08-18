<?php
require_once 'lib/default_config.php';

class sharaclub_config extends default_config
{
    /**
     * @param $command
     * @return string
     */
    private function replace_api_command($command)
    {
        return str_replace(
            array(Plugin_Macros::API_URL, Plugin_Macros::LOGIN, Plugin_Macros::PASSWORD, Plugin_Macros::COMMAND),
            array($this->get_feature(Plugin_Constants::PROVIDER_API_URL),
                $this->get_login(),
                $this->get_password(),
                $command),
            Plugin_Macros::API_URL . "?a=" . Plugin_Macros::COMMAND .
            "&u=" . Plugin_Macros::LOGIN. "-" . Plugin_Macros::PASSWORD . "&source=dune_editor");
    }

    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        if (empty($this->servers)) {
            try {
                $url = $this->replace_api_command('ch_cdn');
                $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);

                if ($content !== false && $content['status'] === '1') {
                    foreach ($content['allow_nums'] as $server) {
                        $this->servers[$server['id']] = $server['name'];
                    }
                    $this->plugin->set_parameter(Ext_Params::M_SERVER_ID, $content['current']);
                    hd_debug_print("Current server: {$content['current']}");
                } else {
                    hd_debug_print("Unable to download servers information");
                }
            } catch (Exception $ex) {
                hd_debug_print("Error during downloading servers information");
                print_backtrace_exception($ex);
            }
        }

        return $this->servers;
    }

    /**
     * @inheritDoc
     */
    public function set_server_id($server)
    {
        try {
            $url = $this->replace_api_command('ch_cdn') . "&num=$server";
            hd_debug_print("change server to: $server", true);
            $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);
            if ($content !== false) {
                hd_debug_print("changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_server_id($server);
                }
            } else {
                hd_debug_print("Unable to change server");
            }
        } catch (Exception $ex) {
            hd_debug_print("Failed to change server");
            print_backtrace_exception($ex);
        }
    }

    /**
     * @inheritDoc
     */
    public function get_profiles()
    {
        $profiles = parent::get_profiles();
        if (empty($profiles)) {
            $profiles = array("0" => TR::t('by_default'));
            try {
                $url = $this->replace_api_command('list_profiles');
                $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);

                if ($content !== false && isset($content['profiles'])) {
                    foreach ($content['profiles'] as $profile) {
                        $profiles[$profile['id']] = $profile['name'];
                    }
                    $profile = isset($content['current']) ? $content['current'] : "0";
                    $this->plugin->set_parameter(Ext_Params::M_PROFILE_ID, $profile);
                    $this->set_profiles($profiles);
                } else {
                    hd_debug_print("Unable to download profiles information");
                }
            } catch (Exception $ex) {
                hd_debug_print("Error during downloading profiles information");
                print_backtrace_exception($ex);
            }
        }

        return $profiles;
    }

    /**
     * @inheritDoc
     */
    public function set_profile_id($profile_id)
    {
        try {
            $url = $this->replace_api_command('list_profiles') . "&num=$profile_id";
            hd_debug_print("change profile to: $profile_id");
            $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url), true);
            if ($content !== false) {
                hd_debug_print("changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_profile_id($profile_id);
                }
            } else {
                hd_debug_print("Unable to change profile");
            }
        } catch (Exception $ex) {
            hd_debug_print("Failed to change profile");
            print_backtrace_exception($ex);
        }
    }

    /**
     * @inheritDoc
     */
    protected function GetPlaylistUrl()
    {
        $url = parent::GetPlaylistUrl();

        $id = $this->get_profile_id();
        $url .= (empty($id) ? "" : "/" . $id);

        return $url;
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

        // this account has special API to get account info
        $login = $this->get_login();
        $password = $this->get_password();

        try {
            if (empty($login) || empty($password)) {
                throw new Exception("Login or password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $url = $this->replace_api_command('subscr_info');
                $content = Curl_Wrapper::decodeJsonResponse(false, Curl_Wrapper::simple_download_content($url));
                if ($content === false || !isset($content->status) || $content->status !== '1') {
                    throw new Exception("Account status unknown. " . HD::get_last_error());
                }
                $this->account_data = $content;

                $this->set_domains(array(0 => $this->account_data->data->listdomain));
                $this->set_domain_id(0);
                $this->set_epg_param(Plugin_Constants::EPG_FIRST,Epg_Params::EPG_DOMAIN, $this->account_data->data->jsonEpgDomain);
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
    public function AddSubscriptionUI(&$defs)
    {
        $account_data = $this->GetAccountInfo( true);
        if ($account_data === false) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), -10);
            return;
        }

        Control_Factory::add_label($defs, TR::t('balance'), $account_data->data->money . ' RUR', -10);
        Control_Factory::add_label($defs, TR::t('tv_screen_subscription'), $account_data->data->money_need . ' RUR', -10);
        $packages = $account_data->data->abon;
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, TR::t('package'), TR::t('no_packages'), 20);
            return;
        }

        foreach ($packages as $package)
        {
            Control_Factory::add_label($defs, TR::t('package'), $package);
        }

        Control_Factory::add_vgap($defs, 20);
    }

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
            $id = '-1';
            if (isset($item->id)) {
                $id = (string)$item->id;
            } else if (isset($item->series_id)) {
                $id = $item->series_id . "_serial";
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

            $age = isset($item->info->adult) && !empty($item->info->adult)  ? "{$item->info->adult}+" : '';
            $age_limit = empty($age) ? array() : array(TR::t('vod_screen_age_limit') => $age);

            $movie = new Movie($movie_id, $this->plugin);
            $movie->set_data(
                $item->name,                          // name,
                '',                       // name_original,
                $item->info->plot,                    // description,
                $item->info->poster,                  // poster_url,
                $duration,                            // length_min,
                $item->info->year,                    // year,
                $item->info->director,                // director_str,
                '',                       // scenario_str,
                $item->info->cast,                    // actors_str,
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
                    $movie->add_season_data($season->season,
                        empty($season->info->name)
                            ? TR::t('vod_screen_season__1', $season->season)
                            : $season->info->name
                        ,'');

                    foreach ($season->episodes as $episode) {
                        hd_debug_print("movie playback_url: $episode->video");
                        $movie->add_series_data($episode->id, TR::t('vod_screen_series__1', $episode->episode), '', $episode->video, $season->season);
                    }
                }
            } else {
                hd_debug_print("movie playback_url: $item->video");
                $movie->add_series_data($movie_id, $item->name, '', $item->video);
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
        $years = array();
        foreach ($this->vod_items as $movie) {
            $movie = (object)$movie;
            $category = (string)$movie->category;
            if (empty($category)) {
                $category = TR::load_string('no_category');
            }

            if (!array_key_exists($category, $cat_info)) {
                $cat_info[$category] = 0;
            }

            ++$cat_info[$category];

            // collect filters information
            $movie_year = (int)$movie->info['year'];
            $years[$movie_year] = $movie_year;
            foreach ($movie->info['genre'] as $genre) {
                $genres[$genre] = $genre;
            }
        }

        foreach ($cat_info as $category => $movie_count) {
            $cat = new Vod_Category($category,
                ($category === Vod_Category::FLAG_ALL_MOVIES) ? TR::t('vod_screen_all_movies__1', " ($movie_count)") : "$category ($movie_count)");
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
        foreach ($this->vod_items as $item) {
            $item = (object)$item;
            $search = utf8_encode(mb_strtolower($item->name, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateShortMovie($item);
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
        foreach ($this->vod_items as $movie) {
            if ($pos++ < $page_idx) continue;

            $movie = (object)$movie;
            $category = $movie->category;
            if (empty($category)) {
                $category = TR::load_string('no_category');
            }

            if ($category_id === Vod_Category::FLAG_ALL_MOVIES || $category_id === $category) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }
        $this->get_next_page($query_id, $pos - $page_idx);

        hd_debug_print("Movies read for query: $query_id - " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($params)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $params");

        if ($this->vod_items === false) {
            hd_debug_print("failed to load movies");
            return array();
        }

        $movies = array();

        $pairs = explode(",", $params);
        $post_params = array();
        foreach ($pairs as $pair) {
            if (preg_match("/^(.+):(.+)$/", $pair, $m)) {
                $filter = $this->get_filter($m[1]);
                if ($filter !== null && !empty($filter['values'])) {
                    $item_idx = array_search($m[2], $filter['values']);
                    if ($item_idx !== false && $item_idx !== -1) {
                        $post_params[$m[1]] = $filter['values'][$item_idx];
                    }
                }
            }
        }

        foreach ($this->vod_items as $movie) {
            $movie = (object)$movie;
            if (isset($post_params['genre'])) {
                $match_genre = in_array($post_params['genre'], $movie->info['genre']);
            } else {
                $match_genre = true;
            }

            $match_year = false;
            $year_from = isset($post_params['from']) ? $post_params['from'] : ~PHP_INT_MAX;
            $year_to = isset($post_params['to']) ? $post_params['to'] : PHP_INT_MAX;

            $movie_year = (int)$movie->info['year'];
            if ($movie_year >= $year_from && $movie_year <= $year_to) {
                $match_year = true;
            }

            if ($match_year && $match_genre) {
                $movies[] = self::CreateShortMovie($movie);
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param Object $movie_obj
     * @return Short_Movie
     */
    protected static function CreateShortMovie($movie_obj)
    {
        $id = '-1';
        if (isset($movie_obj->id)) {
            $id = (string)$movie_obj->id;
        } else if (isset($movie_obj->series_id)) {
            $id = $movie_obj->series_id . "_serial";
        }

        $info = (object)$movie_obj->info;
        $genres = HD::ArrayToStr($info->genre);
        $country = HD::ArrayToStr($info->country);
        return new Short_Movie(
            $id,
            (string)$movie_obj->name,
            (string)$info->poster,
            TR::t('vod_screen_movie_info__5', $movie_obj->name, $info->year, $country, $genres, $info->rating)
        );
    }
}

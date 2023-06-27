<?php
require_once 'lib/default_config.php';

class sharaclub_config extends default_config
{
    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(Plugin_Constants::VOD_FILTER_SUPPORTED, true);
    }

    /**
     * @param $command
     * @param $plugin_cookies
     * @return string
     */
    private function replace_api_command($command, $plugin_cookies)
    {
        return str_replace(
            array(Plugin_Macros::API_URL, Plugin_Macros::LOGIN, Plugin_Macros::PASSWORD, Plugin_Macros::COMMAND),
            array($this->get_feature(Plugin_Constants::PROVIDER_API_URL),
                $this->get_login($plugin_cookies),
                $this->get_password($plugin_cookies),
                $command),
            Plugin_Macros::API_URL . "/api/players.php?a=" . Plugin_Macros::COMMAND .
            "&u=" . Plugin_Macros::LOGIN. "-" . Plugin_Macros::PASSWORD . "&source=dune_editor");
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_servers($plugin_cookies)
    {
        $servers = parent::get_servers($plugin_cookies);
        if (empty($servers)) {
            $servers = array();
            try {
                $url = $this->replace_api_command('ch_cdn', $plugin_cookies);
                $content = HD::DownloadJson($url);

                if ($content !== false && $content['status'] === '1') {
                    foreach ($content['allow_nums'] as $server) {
                        $servers[$server['id']] = $server['name'];
                    }
                    $plugin_cookies->server = $content['current'];
                    hd_print(__METHOD__ . ": Current server: $plugin_cookies->server");
                    $this->set_servers($servers);
                } else {
                    hd_print(__METHOD__ . ": Unable to download servers information");
                }
            } catch (Exception $ex) {
                hd_print(__METHOD__ . ": Error during downloading servers information");
            }
        }

        return $servers;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
        try {
            $url = $this->replace_api_command('ch_cdn', $plugin_cookies) . "&num=$server";
            hd_print(__METHOD__ . ": change server to: $server");
            $content = HD::DownloadJson($url);
            if ($content !== false) {
                hd_print(__METHOD__ . ": changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_server_id($server, $plugin_cookies);
                }
            } else {
                hd_print(__METHOD__ . ": Unable to change server");
            }
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Failed to change server");
        }
    }

    public function get_profiles($plugin_cookies)
    {
        $profiles = parent::get_profiles($plugin_cookies);
        if (empty($profiles)) {
            $profiles = array("0" => TR::t('by_default'));
            try {
                $url = $this->replace_api_command('list_profiles', $plugin_cookies);
                $content = HD::DownloadJson($url);

                if ($content !== false && isset($content['profiles'])) {
                    foreach ($content['profiles'] as $profile) {
                        $profiles[$profile['id']] = $profile['name'];
                    }
                    $plugin_cookies->profile = isset($content['current']) ? $content['current'] : "0";
                    hd_print(__METHOD__ . ": Current profile: {$profiles[$plugin_cookies->profile]} ($plugin_cookies->profile)");
                    $this->set_profiles($profiles);
                } else {
                    hd_print(__METHOD__ . ": Unable to download profiles information");
                }
            } catch (Exception $ex) {
                hd_print(__METHOD__ . ": Error during downloading profiles information");
            }
        }

        return $profiles;
    }

    /**
     * @param $profile
     * @param $plugin_cookies
     */
    public function set_profile_id($profile, $plugin_cookies)
    {
        try {
            $url = $this->replace_api_command('list_profiles', $plugin_cookies) . "&num=$profile";
            hd_print(__METHOD__ . ": change profile to: $profile");
            $content = HD::DownloadJson($url);
            if ($content !== false) {
                hd_print(__METHOD__ . ": changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_profile_id($profile, $plugin_cookies);
                }
            } else {
                hd_print(__METHOD__ . ": Unable to change profile");
            }
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": Failed to change profile");
        }
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($plugin_cookies)
    {
        $url = parent::GetPlaylistUrl($plugin_cookies);

        $id = $this->get_profile_id($plugin_cookies);
        $url .= (empty($id) ? "" : "/" . $id);

        return $url;
    }

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
        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        try {
            if (empty($login) || empty($password)) {
                throw new Exception("Login or password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                hd_print(__METHOD__ . ": Load player configuration");
                $api = HD::DownloadJson(base64_decode("aHR0cDovL2NvbmYucGxheXR2LnByby9hcGkvY29uOGZpZy5waHA/c291cmNlPWR1bmVfZWRpdG9y"), false);
                if ($api === false) {
                    hd_print(__METHOD__ . ": Unable to load player configuration");
                } else {
                    $this->set_feature(Plugin_Constants::PROVIDER_API_URL, "http://$api->listdomain");
                    $this->set_epg_param(Plugin_Constants::EPG_FIRST,Epg_Params::EPG_DOMAIN, "http://$api->jsonEpgDomain");

                    $url = $this->replace_api_command('subscr_info', $plugin_cookies);
                    $json = HD::DownloadJson($url);
                    if ($json === false || !isset($json['status']) || $json['status'] !== '1') {
                        throw new Exception("Account status unknown");
                    }
                    $this->account_data = $json;
                }
            }
        } catch (Exception $ex) {
            hd_print(__METHOD__ . ": " . $ex->getMessage());
            return false;
        }

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
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), -10);
            return;
        }

        $title = 'Пакеты: ';

        Control_Factory::add_label($defs, TR::t('balance'), $account_data['data']['money'] . ' RUR', -10);
        Control_Factory::add_label($defs, TR::t('tv_screen_subscription'), $account_data['data']['money_need'] . ' RUR', -10);
        $packages = $account_data['data']['abon'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, $title, TR::t('no_packages'), 20);
            return;
        }

        foreach ($packages as $package)
        {
            Control_Factory::add_label($defs, $title, $package);
        }

        Control_Factory::add_vgap($defs, 20);
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

            $genres = HD::ArrayToStr($item->info->genre);
            $country = HD::ArrayToStr($item->info->country);

            $movie->set_data(
                $item->name,            // name,
                '',          // name_original,
                $item->info->plot,      // description,
                $item->info->poster,    // poster_url,
                $duration,              // length_min,
                $item->info->year,      // year,
                $item->info->director,  // director_str,
                '',           // scenario_str,
                $item->info->cast,      // actors_str,
                $genres,                // genres_str,
                $item->info->rating,    // rate_imdb,
                '',         // rate_kinopoisk,
                '',            // rate_mpaa,
                $country,               // country,
                ''               // budget
            );

            // case for serials
            if (isset($item->seasons)) {
                foreach ($item->seasons as $season) {
                    $movie->add_season_data($season->season,
                        !empty($season->info->name) ? $season->info->name : TR::t('vod_screen_season__1', $season->season), '');
                    foreach ($season->episodes as $episode) {
                        hd_print(__METHOD__ . ": movie playback_url: $episode->video");
                        $movie->add_series_data($episode->id, TR::t('vod_screen_series__1', $episode->episode), '', $episode->video, $season->season);
                    }
                }
            } else {
                hd_print(__METHOD__ . ": movie playback_url: $item->video");
                $movie->add_series_data($movie_id, $item->name, '', $item->video);
            }

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

        HD::StoreContentToFile(self::get_vod_cache_file(), $jsonItems);

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
            $years[(int)$movie->info->year] = $movie->info->year;
            foreach ($movie->info->genre as $genre) {
                $genres[$genre] = $genre;
            }
        }

        foreach ($cat_info as $category => $movie_count) {
            $cat = new Vod_Category($category,
                ($category === Vod_Category::FLAG_ALL) ? TR::t('vod_screen_all_movies__1', " ($movie_count)") : "$category ($movie_count)");
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

    ///////////////////////////////////////////////////////////////////////

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
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

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
                hd_print(__METHOD__ . ": Filter: $m[1] Value: $m[2]");
                $filter = $this->get_filter($m[1]);
                if ($filter !== null && !empty($filter['values'])) {
                    $item_idx = array_search($m[2], $filter['values']);
                    if ($item_idx !== false && $item_idx !== -1) {
                        $post_params[$m[1]] = $item_idx;
                        hd_print(__METHOD__ . ": Param: $item_idx");
                    }
                }
            }
        }

        foreach ($jsonItems as $movie) {
            $match_genre = !isset($post_params['genre']);
            $info = $movie->info;
            if (!$match_genre) {
                foreach ($info->genre as $genre) {
                    if (!isset($post_params['genre']) || $genre === $post_params['genre']) {
                        $match_genre = true;
                        break;
                    }
                }
            }

            $match_year = false;
            $year_from = isset($post_params['from']) ? $post_params['from'] : ~PHP_INT_MAX;
            $year_to = isset($post_params['to']) ? $post_params['to'] : PHP_INT_MAX;

            if ((int)$info->year >= $year_from && (int)$info->year <= $year_to) {
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
        $id = '-1';
        if (isset($movie_obj->id)) {
            $id = (string)$movie_obj->id;
        } else if (isset($movie_obj->series_id)) {
            $id = $movie_obj->series_id . "_serial";
        }

        $info = $movie_obj->info;
        $genres = HD::ArrayToStr($info->genre);
        $country = HD::ArrayToStr($info->country);
        $movie = new Short_Movie($id, (string)$movie_obj->name, (string)$info->poster);
        $movie->info = TR::t('vod_screen_movie_info__5', $movie_obj->name, $info->year, $country, $genres, $info->rating);

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
        $added = false;
        Control_Factory::add_vgap($defs, 20);
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

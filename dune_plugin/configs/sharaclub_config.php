<?php
require_once 'lib/default_config.php';

class sharaclub_config extends default_config
{
    const API_HOST = "http://conf.playtv.pro/api";

    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(VOD_FILTER_SUPPORTED, true);
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(API_REQUEST_URL, "http://{SUBDOMAIN}/api/players.php?a={COMMAND}&u={LOGIN}-{PASSWORD}&source=dune_editor");
    }

    /**
     * @param $command
     * @param $plugin_cookies
     * @return string
     */
    private function replace_api_command($command, $plugin_cookies)
    {
        return str_replace(
            array('{SUBDOMAIN}', '{LOGIN}', '{PASSWORD}', '{COMMAND}'),
            array($plugin_cookies->subdomain, $this->get_login($plugin_cookies), $this->get_password($plugin_cookies), $command),
            $this->get_feature(API_REQUEST_URL));
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
                    hd_print("Current server: $plugin_cookies->server");
                    $this->set_servers($servers);
                } else {
                    hd_print("Unable to download servers information");
                }
            } catch (Exception $ex) {
                hd_print("Error during downloading servers information");
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
            hd_print("change server to: $server");
            $content = HD::DownloadJson($url);
            if ($content !== false) {
                hd_print("changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_server_id($server, $plugin_cookies);
                }
            } else {
                hd_print("Unable to change server");
            }
        } catch (Exception $ex) {
            hd_print("Failed to change server");
        }
    }

    public function get_profiles($plugin_cookies)
    {
        $profiles = parent::get_profiles($plugin_cookies);
        if (empty($profiles)) {
            $profiles = array();
            try {
                $url = $this->replace_api_command('list_profiles', $plugin_cookies);
                $content = HD::DownloadJson($url);

                if ($content !== false && isset($content['profiles'])) {
                    foreach ($content['profiles'] as $profile) {
                        $profiles[$profile['id']] = $profile['name'];
                    }
                    $plugin_cookies->profile = $content['current'];
                    hd_print("Current server: $plugin_cookies->server");
                    $this->set_profiles($profiles);
                } else {
                    hd_print("Unable to download profiles information");
                }
            } catch (Exception $ex) {
                hd_print("Error during downloading profiles information");
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
            hd_print("change profile to: $profile");
            $content = HD::DownloadJson($url);
            if ($content !== false) {
                hd_print("changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    parent::set_profile_id($profile, $plugin_cookies);
                }
            } else {
                hd_print("Unable to change profile");
            }
        } catch (Exception $ex) {
            hd_print("Failed to change profile");
        }
    }

    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        $url = parent::GetPlaylistUrl($type, $plugin_cookies);
        $profiles = $this->get_profiles($plugin_cookies);
        if (!empty($profiles)) {
            $id = $this->get_profile_id($plugin_cookies);
		    $url .= "/" . $profiles[$id];
	    }

        return $url;
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account: $force");

        // this account has special API to get account info
        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        try {
            if (empty($login) || empty($password)) {
                throw new Exception("Login or password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $api = HD::DownloadJson(self::API_HOST . '/con8fig.php?source=dune_editor', false);

                $plugin_cookies->subdomain = $api->listdomain;
                $this->set_epg_param(EPG_FIRST,EPG_URL, "http://$api->jsonEpgDomain/get/?type=epg&ch={EPG_ID}");

                $url = $this->replace_api_command('subscr_info', $plugin_cookies);
                $json = HD::DownloadJson($url);
                if ($json === false || !isset($json['status']) || $json['status'] !== '1') {
                    throw new Exception("Account status unknown");
                }
                $this->account_data = $json;
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
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
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            Control_Factory::add_label($defs, 'Ошибка!', $text[0], -10);
            Control_Factory::add_label($defs, 'Описание:', $text[1], 20);
            return;
        }

        $title = 'Пакеты: ';

        Control_Factory::add_label($defs, 'Баланс:', $account_data['data']['money'] . ' руб.', -10);
        Control_Factory::add_label($defs, 'Цена подписки:', $account_data['data']['money_need'] . ' руб.', -10);
        $packages = $account_data['data']['abon'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, $title, 'Нет пакетов', 20);
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
        hd_print("TryLoadMovie: $movie_id");
        $movie = new Movie($movie_id);
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());

        if ($jsonItems === false) {
            hd_print("TryLoadMovie: failed to load movie: $movie_id");
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
                    $movie->add_season_data($season->season, !empty($season->info->name) ? $season->info->name : "Сезон $season->season", '');
                    foreach ($season->episodes as $episode) {
                        $playback_url = str_replace("https://", "http://", $episode->video);
                        hd_print("movie playback_url: $playback_url");
                        $movie->add_series_data($episode->id, "Серия $episode->episode", '', $playback_url, $season->season);
                    }
                }
            } else {
                $playback_url = str_replace("https://", "http://", $item->video);
                hd_print("movie playback_url: $playback_url");
                $movie->add_series_data($movie_id, $item->name, '', $playback_url);
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
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $jsonItems = HD::DownloadJson($this->GetVodListUrl($plugin_cookies), false);
        if ($jsonItems === false) {
            return;
        }

        HD::StoreContentToFile($jsonItems, self::get_vod_cache_file());

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $genres = array();
        $years = array();
        foreach ($jsonItems as $movie) {
            $category = (string)$movie->category;
            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }

            // collect filters information
            $info = $movie->info;
            $year = (int)$info->year;
            $years[$year] = $info->year;

            foreach ($info->genre as $genre) {
                $genres[$genre] = $genre;
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

    ///////////////////////////////////////////////////////////////////////

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
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

        foreach ($jsonItems as $item) {
            if ($category_id === $item->category) {
                $movies[] = self::CreateShortMovie($item);
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
                hd_print("Filter: $m[1] Value: $m[2]");
                $filter = $this->get_filter($m[1]);
                if ($filter !== null && !empty($filter['values'])) {
                    $item_idx = array_search($m[2], $filter['values']);
                    if ($item_idx !== false && $item_idx !== -1) {
                        $post_params[$m[1]] = $item_idx;
                        hd_print("Param: $item_idx");
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

        hd_print("Movies found: " . count($movies));
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
        $movie->info = "$movie_obj->name|Год: $info->year|Страна: $country|Жанр: $genres|Рейтинг: $info->rating";

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
        $added = false;
        Control_Factory::add_vgap($defs, 20);
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

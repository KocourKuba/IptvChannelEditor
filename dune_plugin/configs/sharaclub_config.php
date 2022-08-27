<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'tv_url';
    const PLAYLIST_VOD_URL = 'vod_url';
    const ACCOUNT_URL = 'account_url';
    const SERVERS_URL = 'servers_url';
    const API_HOST = "http://conf.playtv.pro/api/con8fig.php?source=dune_editor";

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(VOD_FILTER_SUPPORTED, true);
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/live/(?<token>.+)/(?<id>.+)/.+\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/live/{TOKEN}/{ID}.ts');

        $this->set_epg_param('first','epg_root', '');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $url = $channel->get_streaming_url();
        if (empty($url)) {
            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $template = $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $template = $this->get_feature(MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token']),
                $template);
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf($this->get_feature(self::PLAYLIST_TV_URL), $login, $password);
            case 'movie':
                return sprintf($this->get_feature(self::PLAYLIST_VOD_URL), $login, $password);
        }

        return '';
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);

        $api = HD::DownloadJson(self::API_HOST, false);
        $this->set_feature(self::PLAYLIST_TV_URL, "http://$api->listdomain/tv_live-m3u8/%s-%s");
        $this->set_feature(self::PLAYLIST_VOD_URL, "http://$api->listdomain/kino-full/%s-%s");
        $this->set_feature(self::ACCOUNT_URL, "http://$api->listdomain/api/players.php?a=subscr_info&u=%s-%s&source=dune_editor");
        $this->set_feature(self::SERVERS_URL, "http://$api->listdomain/api/players.php?a=ch_cdn&u=%s-%s&source=dune_editor");
        $this->set_epg_param('first','epg_url', "http://$api->jsonEpgDomain/get/?type=epg&ch={CHANNEL}");

        // this account has special API to get account info
        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if ($force === false && !empty($login) && !empty($password)) {
            return true;
        }

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return false;
        }

        try {
            $url = sprintf($this->get_feature(self::ACCOUNT_URL), $login, $password);
            $account_data = HD::DownloadJson($url);
            if ($account_data === false || !isset($account_data['status']) || $account_data['status'] !== '1')
                return false;
        } catch (Exception $ex) {
            return false;
        }

        return $account_data;
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
        $url = $this->GetPlaylistUrl('movie', $plugin_cookies);
        $jsonItems = HD::DownloadJson($url, false);
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
                $cat = new Starnet_Vod_Category($category, $category);
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
     * @param Starnet_Filter_Screen $parent
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

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        $servers = array();
        try {
            $login = $this->get_login($plugin_cookies);
            $password = $this->get_password($plugin_cookies);
            $url = sprintf($this->get_feature(self::SERVERS_URL), $login, $password);
            $content = HD::DownloadJson($url);

            if ($content !== false && $content['status'] === '1') {
                foreach ($content['allow_nums'] as $server) {
                    $servers[(int)$server['id']] = $server['name'];
                }
                $plugin_cookies->server = $content['current'];
                hd_print("Current server: $plugin_cookies->server");
            } else {
                hd_print("Unable to download servers information");
            }
        } catch (Exception $ex) {
            hd_print("Error during downloading servers information");
        }

        return $servers;
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server($plugin_cookies)
    {
        return isset($plugin_cookies->server) ? $plugin_cookies->server : 0;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server($server, $plugin_cookies)
    {
        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        try {
            $url = sprintf($this->get_feature(self::SERVERS_URL), $login, $password);
            hd_print("change server to: $server");
            $content = HD::DownloadJson($url . "&num=$server");
            if ($content !== false) {
                hd_print("changing result: {$content['msg']}");
                if ($content['status'] === '1') {
                    $plugin_cookies->server = $server;
                }
            } else {
                hd_print("Unable to change server");
            }
        } catch (Exception $ex) {
            hd_print("Failed to change server");
        }
    }
}

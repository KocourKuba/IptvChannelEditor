<?php

require_once 'lib/default_config.php';

class iptvonline_config extends default_config
{
    const API_ACTION_MOVIE = 'movie';
    const API_ACTION_SERIAL = 'serial';
    const API_ACTION_FILTERS = 'filters';
    const API_ACTION_SEARCH = 'search';
    const API_ACTION_FILTER = 'filter';

    const API_COMMAND_GET_PLAYLIST = '{API_URL}/playlist/m3u8';
    const API_COMMAND_GET_VOD = '{API_URL}/catalog';
    const API_COMMAND_ACCOUNT_INFO = '{API_URL}/profile';
    const API_COMMAND_GET_DEVICE = '{API_URL}/device/info';
    const API_COMMAND_SET_DEVICE = '{API_URL}/device/settings';
    const API_COMMAND_REQUEST_TOKEN = '{API_URL}/auth';
    const API_COMMAND_REFRESH_TOKEN = '{API_URL}/oauth2';

    const REQUEST_TEMPLATE = "/movies?limit=100&page=%s&category=%s";

    /**
     * @var Object
     */
    protected $device;

    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        hd_debug_print(null, true);

        if (empty($this->device)) {
            $json = $this->make_json_request(self::API_COMMAND_GET_DEVICE);
            if ($json === false || $json === null) {
                hd_debug_print("failed to load device info");
                return false;
            }

            if (isset($json->status) && $json->status === 200) {
                $this->device = $json;
            }
        }

        $servers = $this->collect_servers($selected);
        if ($selected !== $this->plugin->get_credentials(Ext_Params::M_SERVER_ID)) {
            $this->plugin->set_credentials(Ext_Params::M_SERVER_ID, $selected);
        }
        return $servers;
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("Collect information from account: " . var_export($force, true));

        $json = $this->make_json_request(self::API_COMMAND_ACCOUNT_INFO);
        if ($json === false || $json === null) {
            hd_debug_print("failed to load account info");
            return false;
        }

        return $this->account_data = $json;
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);

        // movies_84636 or serials_84636
        $arr = explode("_", $movie_id);
        hd_debug_print("TryLoadMovie: category: movies, id: $arr[1]");

        $params[CURLOPT_CUSTOMREQUEST] = "/movies/$arr[1]";
        $json = $this->make_json_request(self::API_COMMAND_GET_VOD, $params);

        if ($json === false || $json === null) {
            hd_debug_print("failed to load movie: $movie_id");
            return null;
        }

        $movie = new Movie($movie_id, $this->plugin);
        $movieData = $json->data;
        if ($arr[0] === self::API_ACTION_MOVIE) {
            $movie_serie = new Movie_Series($arr[1], $movieData->medias->title, $movieData->medias->url);
            foreach ($movieData->medias->audios as $item) {
                $key = $item->translate;
                $movie_serie->audios[$key] = new Movie_Variant($key, $key, $item->url);
            }
            $movie->add_series_data($movie_serie);
        } else if ($arr[0] === self::API_ACTION_SERIAL) {
            // collect series
            foreach ($movieData->seasons as $season) {
                $movie_season = new Movie_Season($season->season);
                if (!empty($season->title)) {
                    $movie_season->description = $season->title;
                }
                $movie->add_season_data($movie_season);

                foreach ($season->episodes as $episode) {
                    hd_debug_print("movie playback_url: $episode->url");

                    $audios = array();
                    foreach ($episode->audios as $item) {
                        $key = $item->translate;
                        $audios[$key] = new Movie_Variant($key, $key, $item->url);
                    }

                    $movie_serie = new Movie_Series("$season->season:$episode->episode",
                        TR::t('vod_screen_series__1', $episode->episode),
                        $episode->url,
                        $season->season
                    );
                    $movie_serie->description = $episode->title;
                    $movie_serie->audios = $audios;
                    $movie->add_series_data($movie_serie);
                }
            }
        }

        $movie->set_data(
            $movieData->ru_title,                     // caption,
            $movieData->orig_title,                   // caption_original,
            $movieData->plot,                         // description,
            $movieData->posters->big,                 // poster_url,
            $movieData->duration / 60,      // length,
            $movieData->year,                         // year,
            $movieData->director,                     // director,
            '',                           // scenario,
            $movieData->cast,                         // actors,
            self::collect_genres($movieData),         // genres,
            $movieData->imdb_rating,                  // rate_imdb,
            $movieData->kinopoisk_rating,             // rate_kinopoisk,
            '',                             // rate_mpaa,
            self::collect_countries($movieData),      // country,
            '',                                // budget
            array(),                                  // details
            array(TR::t('quality') => $movieData->quality)   // rate details
        );

        return $movie;
    }


    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        hd_debug_print(null, true);

        $category_list = array();
        $category_index = array();

        $cat = new Vod_Category(self::API_ACTION_MOVIE, TR::t('vod_screen_all_movies'));
        $category_list[] = $cat;
        $category_index[$cat->get_id()] = $cat;
        $cat = new Vod_Category(self::API_ACTION_SERIAL, TR::t('vod_screen_all_serials'));
        $category_list[] = $cat;
        $category_index[$cat->get_id()] = $cat;

        $exist_filters = array();
        $params[CURLOPT_CUSTOMREQUEST] = '/' . self::API_ACTION_FILTERS;
        $data = $this->make_json_request(self::API_COMMAND_GET_VOD, $params);
        if (!isset($data->success, $data->data->filter_by) || !$data->success) {
            hd_debug_print("Wrong response on filter request: " . json_encode($data), true);
            return false;
        }

        $exist_filters['source'] = array(
            'title' => TR::load('category'),
            'values' => array(
                self::API_ACTION_MOVIE => TR::load('vod_screen_all_movies'),
                self::API_ACTION_SERIAL => TR::load('vod_screen_all_serials')
            )
        );

        foreach ($data->data->filter_by as $filter) {
            if (!isset($filter->id)) continue;

            if (empty($filter->items)) {
                $exist_filters[$filter->id] = array('title' => $filter->title, 'text' => true);
            } else {
                $exist_filters[$filter->id] = array('title' => $filter->title, 'values' => array(-1 => TR::t('no')));
                foreach ($filter->items as $item) {
                    if ($item->enabled) {
                        $exist_filters[$filter->id]['values'][$item->id] = $item->title;
                    }
                }
            }
        }

        $this->set_filters($exist_filters);

        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Filters count: " . count($exist_filters));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print(null, true);
        hd_debug_print("getSearchList $keyword");

        $params[CURLOPT_POSTFIELDS] = array("search" => $keyword);

        $movies = array();
        $page_id = self::API_ACTION_MOVIE . "_" . self::API_ACTION_SEARCH;
        $page_idx = $this->get_next_page($page_id);
        if ($page_idx < 0)
            return $movies;

        $params[CURLOPT_CUSTOMREQUEST] = sprintf(self::REQUEST_TEMPLATE, $page_idx, self::API_ACTION_MOVIE);
        $searchRes = $this->make_json_request(self::API_COMMAND_GET_VOD, $params);

        $movies = ($searchRes === false) ? array() : $this->CollectSearchResult(self::API_ACTION_MOVIE, $searchRes, self::API_ACTION_SEARCH);

        $page_id = self::API_ACTION_SERIAL . "_" . self::API_ACTION_SEARCH;
        $page_idx = $this->get_next_page($page_id);
        if ($page_idx < 0)
            return $movies;

        $params[CURLOPT_CUSTOMREQUEST] = sprintf(self::REQUEST_TEMPLATE, $page_idx, self::API_ACTION_SERIAL);
        $searchRes = $this->make_json_request(self::API_COMMAND_GET_VOD, $params);
        $serials = ($searchRes === false) ? array() : $this->CollectSearchResult(self::API_ACTION_SERIAL, $searchRes, self::API_ACTION_SEARCH);

        return array_merge($movies, $serials);
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($params)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $params");

        $pairs = explode(",", $params);
        $filter_params = array();
        foreach ($pairs as $pair) {
            // country:USA
            // genre:action
            // year:2024
            /** @var array $m */
            if (!preg_match("/^(.+):(.+)$/", $pair, $m)) continue;

            $filter = $this->get_filter($m[1]);
            if ($filter === null) continue;

            if (isset($filter['text'])) {
                $filter_params[$m[1]] = $m[2];
            } else if (!empty($filter['values'])) {
                $item_idx = array_search($m[2], $filter['values']);
                if ($item_idx !== false && $item_idx !== -1) {
                    $filter_params[$m[1]] = $item_idx;
                }
            }
        }

        if (empty($filter_params)) {
            return false;
        }

        $param_str = '';
        $query_id = self::API_ACTION_MOVIE;
        foreach ($filter_params as $key => $value) {
            if ($key === 'source') {
                $query_id = $value;
                continue;
            }

            if ($key === 'year') {
                $values = explode('-', $value);
                if (count($values) === 1) {
                    $value = "$value-$value";
                }
            }

            if (!empty($param_str)) {
                $param_str .= "_";
            }
            $param_str .= "$key-$value";
        }

        $page_id = $query_id . "_" . self::API_ACTION_FILTER;
        $page_idx = $this->get_next_page($page_id);
        if ($page_idx < 0)
            return array();

        hd_debug_print("filter page_idx:  $page_idx");

        $post_params[CURLOPT_CUSTOMREQUEST] = sprintf(self::REQUEST_TEMPLATE, $page_idx, $query_id);
        $post_params[CURLOPT_POSTFIELDS] = array('features_hash' => $param_str);
        $json = $this->make_json_request(self::API_COMMAND_GET_VOD, $post_params);

        return $json === false ? array() : $this->CollectSearchResult($query_id, $json, self::API_ACTION_FILTER);
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        $page_idx = $this->get_next_page($query_id);
        $params[CURLOPT_CUSTOMREQUEST] = sprintf(self::REQUEST_TEMPLATE, $page_idx, $query_id);
        $json = $this->make_json_request(self::API_COMMAND_GET_VOD, $params);

        return ($json === false || $json === null) ? array() : $this->CollectSearchResult($query_id, $json);
    }

    /**
     * @return string|bool
     */
    protected function GetPlaylistUrl()
    {
        $json = $this->make_json_request(self::API_COMMAND_GET_PLAYLIST);
        if ($json === false || !isset($json->data)) {
            hd_debug_print("failed to load account info");
            return false;
        }

        return $json->data;
    }

    /**
     * @param string $query_id
     * @param Object $json
     * @param $search string|null
     * @return array
     */
    protected function CollectSearchResult($query_id, $json, $search = null)
    {
        hd_debug_print(null, true);
        hd_debug_print("query_id: $query_id");

        $movies = array();
        if (!isset($json->data->items))
            return $movies;

        $page_id = is_null($search) ? $query_id : "{$query_id}_$search";
        $current_idx = $this->get_current_page($page_id);
        if ($current_idx < 0)
            return $movies;

        $data = $json->data;
        foreach ($data->items as $entry) {
            $movie = new Short_Movie(
                "{$query_id}_$entry->id",
                $entry->ru_title,
                $entry->posters->medium,
                TR::t('vod_screen_movie_info__4', $entry->ru_title, $entry->year, self::collect_countries($entry), self::collect_genres($entry))
            );

            $movie->big_poster_url = $entry->posters->big;
            $movies[] = $movie;
        }

        if ($data->pagination->pages === $current_idx) {
            hd_debug_print("Last page: {$data->pagination->pages}");
            $this->set_next_page($page_id, -1);
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $path
     * @param array|null $params
     * @return bool|object
     */
    protected function make_json_request($path, $params = null)
    {
        if (!$this->ensure_token_loaded()) {
            return false;
        }

        $curl_opt = array();

        if (isset($params[CURLOPT_CUSTOMREQUEST])) {
            $curl_opt[CURLOPT_CUSTOMREQUEST] = $params[CURLOPT_CUSTOMREQUEST];
        }

        $curl_opt[CURLOPT_HTTPHEADER][] = CONTENT_TYPE_JSON;
        $curl_opt[CURLOPT_HTTPHEADER][] = AUTH_BEARER . $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);

        if (isset($params[CURLOPT_POSTFIELDS])) {
            $curl_opt[CURLOPT_POSTFIELDS] = $params[CURLOPT_POSTFIELDS];
        }

        $data = $this->execApiCommand($path, null, true, $curl_opt);
        if ((isset($data->success) && !$data->success) || (isset($data->status) && $data->status !== 200)) {
            hd_debug_print("Wrong response: " . json_encode($data));
            return false;
        }

        return $data;
    }

    protected static function collect_countries($entry)
    {
        $countries_str = '';
        if (isset($entry->countries)) {
            $countries = array();
            foreach ($entry->countries as $country) {
                if (!empty($country)) {
                    $countries[] = $country;
                }
            }
            $countries_str = implode(", ", $countries);
        }

        return $countries_str;
    }

    protected static function collect_genres($entry)
    {
        $genres_str = '';
        if (isset($entry->genres)) {
            $genres = array();
            foreach ($entry->genres as $genre) {
                if (!empty($genre)) {
                    $genres[] = $genre;
                }
            }
            $genres_str = implode(", ", $genres);
        }

        return $genres_str;
    }

    /**
     * @inheritDoc
     */
    protected function ensure_token_loaded($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("force request provider token: " . var_export($force, true));

        $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
        $expired = time() > (int)$this->plugin->get_credentials(Ext_Params::M_EXPIRE_DATA);
        if (!$force && !empty($token) && !$expired) {
            hd_debug_print("request not required", true);
            return true;
        }

        $refresh_token = $this->plugin->get_credentials(Ext_Params::M_R_TOKEN);
        $refresh = $expired && !empty($refresh_token);
        if ($refresh) {
            /*
            {
                "client_id" : "{{client_id}}",
                "client_secret": "{{client_secret}}",
                "device_id": "{{DEVICE_UNIQ_ID}}",
                "grant_type" : "refresh_token",
                "refresh_token" : "{{refresh_token}}",
            }
            */
            hd_debug_print("need to refresh token", true);
            $cmd = self::API_COMMAND_REFRESH_TOKEN;
            $pairs['grant_type'] = 'refresh_token';
            $pairs['refresh_token'] = $refresh_token;
        } else {
            /*
            {
                "client_id" : "{{client_id}}",
                "client_secret": "{{client_secret}}",
                "device_id": "{{DEVICE_UNIQ_ID}}".
                "login": "{{TEST_EMAIL_CLIENT}}",
                "password" : "{{TEST_EMAIL_CLIENT_PASSWORD}}"
            }
            */
            hd_debug_print("need to request token", true);
            $cmd = self::API_COMMAND_REQUEST_TOKEN;
            $pairs['login'] = $this->get_login();
            $pairs['password'] = $this->get_password();
        }

        $pairs['client_id'] = "TestAndroidAppV0";
        $pairs['client_secret'] = "kshdiouehruyiwuresuygr736t4763b7637"; // dummy
        $pairs['device_id'] = get_serial_number();

        $curl_opt[CURLOPT_POST] = true;
        $curl_opt[CURLOPT_HTTPHEADER][] = CONTENT_TYPE_JSON;
        $curl_opt[CURLOPT_POSTFIELDS] = $pairs;

        $data = $this->execApiCommand($cmd, null, true, $curl_opt);
        if (isset($data->access_token)) {
            hd_debug_print("token requested", true);
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $data->access_token);
            $this->plugin->set_credentials(Ext_Params::M_R_TOKEN, $data->refresh_token);
            $this->plugin->set_credentials(Ext_Params::M_EXPIRE_DATA, $data->expires_time);
            hd_debug_print("s_token: $data->access_token", true);
            hd_debug_print("r_token: $data->refresh_token", true);
            hd_debug_print("expired: $data->expires_time", true);
            return true;
        }

        hd_debug_print("token not received: " . json_encode($data), true);
        return false;
    }

    /**
     * collect servers information
     * @param string $selected
     * @return array
     */
    protected function collect_servers(&$selected = "-1")
    {
        $servers = array();
        if (isset($this->device->device->settings->server_location->value)) {
            foreach ($this->device->device->settings->server_location->value as $server) {
                $servers[(string)$server->id] = $server->label;
                if ($server->selected) {
                    $selected = (string)$server->id;
                }
            }
        }

        return $servers;
    }
}

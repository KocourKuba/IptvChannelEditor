<?php
require_once 'lib/default_config.php';

/**
 * TorrentTV. API v3 https://tapitv.com/v3/api_v3.html
 * Account is set by login/password pair: auth returns the API session,
 * the playlist token (S_TOKEN macro of the playlist url) is taken from the account info.
 * VOD lists are paginated by server, categories are the genres (merged genres of the films, serials and animation sections)
 * or the types (films, serials, animation) selected by the category mode from the category list popup menu.
 * Type category is served by film_filter, film_list ignores the type.
 * film_list serves the whole catalog, the genre category, the genre only filter and the search (genre/search params),
 * but ignores the type, country and years, so other filters are served by film_filter.
 * Search shows all found films: the search screen requests the list once, so all pages are loaded at once.
 * film_alt_search ignores pagination and cuts the result at 50 films, it is used only when film_list fails.
 * Movie files (film_alt_http) are HLS streams, files of the same movie are selectable qualities,
 * series files contain season and episode numbers.
 */
class ttv_config extends default_config
{
    const API_COMMAND_AUTH = '{API_URL}/auth.php?username=%s&password=%s&application=android&guid=%s';
    const API_COMMAND_URL = '{API_URL}/%s.php?session=%s';
    // film_list returns the poster path relative to the media root of the poster host
    const POSTER_URL_PREFIX = 'http://mdb.ttvrun.one/media/';
    // search result limit: SEARCH_MAX_PAGES * PAGE_SIZE films
    const SEARCH_MAX_PAGES = 10;

    // credentials parameter of the saved session: hash|session
    const PARAM_SESSION = 'ttv_session';
    // error code of the expired or revoked session, new session must be requested
    const ERROR_INCORRECT = 'incorrect';
    // session does not expire while in use, refresh it once a week
    const SESSION_LIFETIME = 604800;
    const PAGE_SIZE = 50;

    const TYPE_ALL = 'all';
    const TYPE_FILM = 'film';
    const TYPE_SERIAL = 'serial';
    const TYPE_ANIMATION = 'animation';

    const CATEGORY_MODE_GENRE = 'genre';
    const CATEGORY_MODE_TYPE = 'type';

    /**
     * film_filter types shown as categories in the type category mode
     *
     * @var string[]
     */
    protected static $category_types = array(self::TYPE_FILM, self::TYPE_SERIAL, self::TYPE_ANIMATION);

    /**
     * @var string
     */
    protected $session = '';

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

        if (!$force && !empty($this->account_data)) {
            return $this->account_data;
        }

        $info = $this->make_json_request('userinfo');
        if ($info === false) {
            hd_debug_print("Failed to load account info");
            return false;
        }

        $this->account_data = $info;
        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function AddSubscriptionUI(&$defs)
    {
        $account_data = $this->GetAccountInfo(true);
        if ($account_data === false) {
            hd_debug_print("Can't get account status");
            Control_Factory::add_label($defs, TR::t('err_error'), TR::t('warn_msg4'), -10);
            Control_Factory::add_label($defs, TR::t('description'), TR::t('warn_msg5'), 20);
            return;
        }

        Control_Factory::add_label($defs, TR::t('login'), safe_get_value($account_data, 'email', ''), -10);
        Control_Factory::add_label($defs, TR::t('balance'), safe_get_value($account_data, 'balance', 0), -10);
        Control_Factory::add_label($defs, TR::t('package'), 'VIP: ' . (safe_get_value($account_data, 'vip_status') ? TR::load('yes') : TR::load('no')), -10);

        $tsproxy_date = (int)safe_get_value($account_data, 'tsproxy_date', 0);
        if ($tsproxy_date > 0) {
            $tariff = safe_get_value($account_data, 'tsproxy_tariff');
            Control_Factory::add_label($defs, TR::t('end_date'), (empty($tariff) ? '' : "$tariff - ") . date('j.m.Y', $tsproxy_date), -10);
        }

        Control_Factory::add_vgap($defs, 20);
    }

    /**
     * @inheritDoc
     */
    public function get_category_modes()
    {
        return array(
            self::CATEGORY_MODE_GENRE => TR::load('genre'),
            self::CATEGORY_MODE_TYPE => TR::load('vod_filter_type'),
        );
    }

    /**
     * @inheritDoc
     */
    public function get_category_mode()
    {
        $mode = $this->plugin->get_setting(PARAM_VOD_CATEGORY_MODE, self::CATEGORY_MODE_GENRE);
        return $mode === self::CATEGORY_MODE_TYPE ? self::CATEGORY_MODE_TYPE : self::CATEGORY_MODE_GENRE;
    }

    /**
     * @inheritDoc
     */
    public function set_category_mode($mode)
    {
        $this->plugin->set_setting(PARAM_VOD_CATEGORY_MODE, $mode);
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        hd_debug_print(null, true);

        $category_list = array();
        $category_index = array();

        $response = $this->make_json_request('film_alt_genres');
        if ($response === false) {
            hd_debug_print("Failed to load VOD categories");
            return false;
        }

        // in the type mode the categories are the film_filter types with their film count,
        // the genres are still collected for the genre filter
        $by_type = $this->get_category_mode() === self::CATEGORY_MODE_TYPE;
        if ($by_type) {
            foreach (self::$category_types as $type) {
                $caption = TR::load("vod_filter_type_$type");
                $count = $this->get_type_film_count($type);
                if ($count >= 0) {
                    $caption .= " ($count)";
                }

                $category = new Vod_Category($type, $caption);
                $category_list[] = $category;
                $category_index[$category->get_id()] = $category;
            }
        }

        // the sections (film, serial, animation) share the same genre ids, so they are merged
        // into one list: in the genre mode every genre is a category, genres are values of the genre filter.
        // 'All genres' entry without genre id and '-1' user favorites pseudo genre are skipped
        $genre_values = array(-1 => TR::load('no'));
        foreach (safe_get_value($response, 'data', array()) as $section) {
            foreach (safe_get_value($section, 'genres', array()) as $genre) {
                $genre_id = (int)safe_get_value($genre, array('query', 'genre_id'), 0);
                $genre_name = safe_get_value($genre, 'name');
                if ($genre_id <= 0 || empty($genre_name) || isset($genre_values[$genre_id])) continue;

                if (!$by_type) {
                    $category = new Vod_Category((string)$genre_id, $genre_name);
                    $category_list[] = $category;
                    $category_index[$category->get_id()] = $category;
                }
                $genre_values[$genre_id] = $genre_name;
            }
        }

        $exist_filters = array(
            'type' => array(
                'title' => TR::load('vod_filter_type'),
                'values' => array(
                    -1 => TR::load('no'),
                    self::TYPE_ALL => TR::load('vod_filter_type_all'),
                    self::TYPE_FILM => TR::load('vod_filter_type_film'),
                    self::TYPE_SERIAL => TR::load('vod_filter_type_serial'),
                    self::TYPE_ANIMATION => TR::load('vod_filter_type_animation'),
                )
            ),
            'genre' => array('title' => TR::load('genre'), 'values' => $genre_values),
            'from' => array('title' => TR::load('year_from'), 'text' => true),
            'to' => array('title' => TR::load('year_to'), 'text' => true),
        );

        $countries = $this->make_json_request('film_filter_countries');
        if ($countries !== false) {
            $country_values = array(-1 => TR::load('no'));
            foreach (safe_get_value($countries, 'data', array()) as $country) {
                if (!empty($country)) {
                    $country_values[$country] = $country;
                }
            }
            $exist_filters['country'] = array('title' => TR::load('country'), 'values' => $country_values);
        }

        $this->set_filter_types($exist_filters);

        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Filters count: " . count($exist_filters));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print("getMovieList: $query_id");

        // type category: films of the type, film_list ignores the type so it is served by film_filter
        if (in_array($query_id, self::$category_types, true)) {
            return $this->get_page_movies($query_id, array('type' => $query_id));
        }

        // genre category: films of all types of this genre, without genre the whole catalog
        $params = array();
        if ((int)$query_id > 0) {
            $params['genre'] = (int)$query_id;
        }

        return $this->get_page_movies($query_id, $params, 'film_list');
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print(null, true);
        hd_debug_print("getSearchList: $keyword");

        // search screen requests the list only once, so all pages are loaded here
        $movies = array();
        for ($page = 1; $page <= self::SEARCH_MAX_PAGES; $page++) {
            $response = $this->make_json_request('film_list', array('search' => $keyword, 'page' => $page, 'page_size' => self::PAGE_SIZE));
            if ($response === false) {
                break;
            }

            $data = safe_get_value($response, 'data', array());
            $films = safe_get_value($data, 'films', array());
            foreach ($films as $item) {
                $this->CreateShortMovie($item, $movies);
            }

            if (empty($films) || $page >= (int)safe_get_value($data, 'total_pages', 0)) {
                break;
            }
        }

        if (empty($movies) && $page === 1) {
            // film_list failed, film_alt_search returns up to 50 films without pagination
            $response = $this->make_json_request('film_alt_search', array('q' => $keyword));
            if ($response !== false) {
                foreach (safe_get_value($response, 'data', array()) as $item) {
                    $this->CreateShortMovie($item, $movies);
                }
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $query_id");

        // type:Films,genre:Comedy,country:USA,from:2000
        $params = array();
        foreach (explode(',', $query_id) as $pair) {
            /** @var array $m */
            if (!preg_match("/^([^:]+):(.+)$/", $pair, $m)) continue;

            $filter = $this->get_filter($m[1]);
            if ($filter === null) continue;

            if (isset($filter['text'])) {
                $year = (int)$m[2];
                if ($year >= 1900 && $year <= 2100) {
                    $params[$m[1] === 'from' ? 'year_from' : 'year_to'] = $year;
                }
                continue;
            }

            $item_key = array_search($m[2], $filter['values']);
            if ($item_key === false || $item_key === -1) continue;

            switch ($m[1]) {
                case 'type':
                    $params['type'] = $item_key;
                    break;
                case 'genre':
                    $params['genres_include[]'] = $item_key;
                    break;
                case 'country':
                    $params['countries_include[]'] = $item_key;
                    break;
            }
        }

        if (empty($params)) {
            return array();
        }

        // genre only filter is served by film_list like the genre category: it returns all films of the genre.
        // film_list ignores type, country and years, so any other filter is served by film_filter
        $other = array_diff_key($params, array('genres_include[]' => true, 'type' => true));
        $type = safe_get_value($params, 'type', self::TYPE_ALL);
        if (isset($params['genres_include[]']) && $type === self::TYPE_ALL && empty($other)) {
            return $this->get_page_movies($query_id, array('genre' => $params['genres_include[]']), 'film_list');
        }

        return $this->get_page_movies($query_id, $params);
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);

        $response = $this->make_json_request('film_alt_http', array('film_id' => $movie_id));
        $files = safe_get_value($response, 'data', array());
        if (empty($files)) {
            hd_debug_print("Failed to load movie files: $movie_id");
            return null;
        }

        $info = $this->make_json_request('film_alt_info', array('film_id' => $movie_id));
        $film = safe_get_value($info, 'data', array());

        $movie = new Movie($movie_id, $this->plugin);
        $name = safe_get_value($film, 'name', TR::t('no_title'));

        // season is false for the movie
        $seasons = array();
        foreach ($files as $file) {
            $season = safe_get_value($file, 'season', false);
            if ($season !== false) {
                $seasons[(int)$season][(int)safe_get_value($file, 'episode', 0)][] = $file;
            }
        }

        if (empty($seasons)) {
            $movie_series = new Movie_Series($movie_id, $name, safe_get_value($files[0], 'url'));
            $movie_series->qualities = self::collect_qualities($files);
            $movie->add_series_data($movie_series);
        } else {
            ksort($seasons);
            foreach ($seasons as $season_id => $episodes) {
                $movie->add_season_data(new Movie_Season($season_id));

                ksort($episodes);
                foreach ($episodes as $episode => $episode_files) {
                    $first = $episode_files[0];
                    $series_id = safe_get_value($first, 'tfile_id', "{$season_id}_$episode");
                    $movie_series = new Movie_Series($series_id, TR::t('vod_screen_series__1', $episode), safe_get_value($first, 'url'), $season_id);
                    $movie_series->description = safe_get_value($first, 'name', '');
                    $movie_series->qualities = self::collect_qualities($episode_files);
                    $movie->add_series_data($movie_series);
                }
            }
        }

        $qualities = array();
        foreach ($files as $file) {
            $quality = self::make_quality_name($file);
            if (!empty($quality)) {
                $qualities[$quality] = true;
            }
        }

        $details = array();
        if (!empty($qualities)) {
            $details[TR::t('vod_screen_quality')] = implode(', ', array_keys($qualities));
        }

        $movie->set_data(
            $name,// caption,
            '',// caption_original,
            safe_get_value($film, 'description', ''),// description,
            safe_get_value($film, 'poster', ''),// poster_url,
            (int)round((int)safe_get_value($files[0], 'duration', 0) / 60),// length,
            safe_get_value($film, 'year', ''),// year,
            self::get_persons($film, 'ежисс'),// director,
            self::get_persons($film, 'ценар'),// scenario,
            implode(', ', safe_get_value($film, 'actors', array())),// actors,
            implode(', ', safe_get_value($film, 'genres', array())),// genres,
            safe_get_value($film, 'imdb_rating', ''),// rate_imdb,
            safe_get_value($film, 'kp_rating', ''),// rate_kinopoisk,
            '',// rate_mpaa,
            safe_get_value($film, 'country', ''),// country,
            '',// budget
            $details// details
        );

        return $movie;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * Login by login/password pair and get the playlist token.
     * Session is saved and reused if it was made with the same credentials
     *
     * @inheritDoc
     */
    protected function ensure_token_loaded($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("force request provider token: " . var_export($force, true));

        $login = $this->get_login();
        $password = $this->get_password();
        if (empty($login) || empty($password)) {
            hd_debug_print("Login or password not set");
            return false;
        }

        // credentials may be changed in setup
        $hash = hash('crc32', $login . $password);
        list($saved_hash, $session) = explode('|', $this->plugin->get_credentials(self::PARAM_SESSION)) + array('', '');
        $expired = time() > (int)$this->plugin->get_credentials(Ext_Params::M_EXPIRE_DATA);
        $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
        if (!$force && !$expired && $saved_hash === $hash && !empty($session) && !empty($token)) {
            $this->session = $session;
            return true;
        }

        $this->session = '';
        $this->plugin->set_credentials(self::PARAM_SESSION, '');

        $url = sprintf(self::API_COMMAND_AUTH, rawurlencode($login), rawurlencode($password), self::get_guid());
        $response = $this->exec_json($url);
        $session = safe_get_value($response, 'session');
        if ((int)safe_get_value($response, 'success') !== 1 || empty($session)) {
            hd_debug_print("Login failed: " . safe_get_value($response, 'error'));
            return false;
        }

        // playlist token is returned only by the account info
        $info = $this->exec_json(sprintf(self::API_COMMAND_URL, 'userinfo', $session));
        $token = safe_get_value($info, 'playlist_token');
        if ((int)safe_get_value($info, 'success') !== 1 || empty($token)) {
            hd_debug_print("Failed to get playlist token: " . safe_get_value($info, 'error'));
            return false;
        }

        $this->session = $session;
        $this->account_data = $info;
        $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $token);
        $this->plugin->set_credentials(Ext_Params::M_EXPIRE_DATA, time() + self::SESSION_LIFETIME);
        $this->plugin->set_credentials(self::PARAM_SESSION, "$hash|$session");
        hd_debug_print("session requested", true);

        return true;
    }

    /**
     * Execute API command. Session can be revoked by server at any time ('incorrect' error),
     * in this case new session is requested and command is repeated once
     *
     * @param string $command
     * @param array $params
     * @return array|false
     */
    protected function make_json_request($command, $params = array())
    {
        for ($attempt = 0; $attempt < 2; $attempt++) {
            if (!$this->ensure_token_loaded($attempt !== 0)) {
                break;
            }

            $url = sprintf(self::API_COMMAND_URL, $command, $this->session);
            foreach ($params as $key => $value) {
                $url .= "&$key=" . rawurlencode($value);
            }

            $response = $this->exec_json($url);
            if ((int)safe_get_value($response, 'success') === 1) {
                return $response;
            }

            $error = safe_get_value($response, 'error');
            hd_debug_print("Bad response on $command: $error");
            if ($error !== self::ERROR_INCORRECT) {
                break;
            }
        }

        return false;
    }

    /**
     * @param string $url
     * @return array|false
     */
    protected function exec_json($url)
    {
        return Curl_Wrapper::decodeJsonResponse(false, $this->execApiCommand($url, null, false), true);
    }

    /**
     * Load next page of the movies list
     *
     * @param string $page_id
     * @param array $params
     * @return array
     */
    protected function get_page_movies($page_id, $params, $command = 'film_filter')
    {
        $movies = array();

        // film_filter and film_list page is 1 based
        $page = $this->get_next_page($page_id);
        if ($page < 0) {
            return $movies;
        }

        $params['page'] = $page;
        $params['page_size'] = self::PAGE_SIZE;
        $response = $this->make_json_request($command, $params);
        $data = safe_get_value($response, 'data', array());

        // film_filter: data.result and data.pages, film_list: data.films and data.total_pages
        $is_list = ($command === 'film_list');
        foreach (safe_get_value($data, $is_list ? 'films' : 'result', array()) as $item) {
            $this->CreateShortMovie($item, $movies);
        }

        if (empty($movies) || $page >= (int)safe_get_value($data, $is_list ? 'total_pages' : 'pages', 0)) {
            // last page
            $this->set_next_page($page_id, -1);
        }

        hd_debug_print("Movies read for query: $page_id page: $page: " . count($movies));
        return $movies;
    }

    /**
     * Number of films of the film_filter type: data.count of the response,
     * one film per page keeps the response small.
     *
     * @param string $type
     * @return int film count or -1 if the request failed
     */
    protected function get_type_film_count($type)
    {
        $response = $this->make_json_request('film_filter', array('type' => $type, 'page' => 1, 'page_size' => 1));
        if ($response === false) {
            hd_debug_print("Failed to get films count for type: $type");
            return -1;
        }

        return (int)safe_get_value($response, array('data', 'count'), 0);
    }

    /**
     * @param array $movie_info
     * @param array $movies
     * @return void
     */
    protected function CreateShortMovie($movie_info, &$movies)
    {
        $id = safe_get_value($movie_info, 'id');
        // film_list has the russian name in 'rus_name'
        $name = safe_get_value($movie_info, 'rus_name');
        if (empty($name)) {
            $name = safe_get_value($movie_info, 'name');
        }

        if (empty($id) || empty($name)) {
            return;
        }

        $rating = safe_get_value($movie_info, 'kp_rating');
        if (empty($rating)) {
            $rating = safe_get_value($movie_info, 'imdb_rating', '');
        }

        // film_filter gives the absolute 'poster' url, film_list the 'img' path relative to the poster host
        $poster = safe_get_value($movie_info, 'poster', '');
        if (empty($poster)) {
            $poster = safe_get_value($movie_info, 'img', '');
            if (!empty($poster) && strpos($poster, 'http') !== 0) {
                $poster = self::POSTER_URL_PREFIX . ltrim($poster, '/');
            }
        }

        $movie = new Short_Movie((string)$id, $name, $poster,
            TR::t('vod_screen_movie_info__5',
                $name,
                safe_get_value($movie_info, 'year', ''),
                safe_get_value($movie_info, 'country', ''),
                '',
                $rating)
        );

        $this->plugin->vod->set_cached_short_movie($movie);

        $movies[] = $movie;
    }

    /**
     * Files of the movie or episode as qualities, the files of the same quality are distinguished by the file name
     *
     * @param array $files
     * @return Movie_Variant[]|null
     */
    protected static function collect_qualities($files)
    {
        if (count($files) < 2) {
            return null;
        }

        $qualities = array();
        foreach ($files as $file) {
            $url = safe_get_value($file, 'url');
            if (empty($url)) continue;

            $name = self::make_quality_name($file);
            if (empty($name) || isset($qualities[$name])) {
                $name = safe_get_value($file, 'name', $url);
            }
            $qualities[$name] = new Movie_Variant(safe_get_value($file, 'tfile_id', $name), $name, $url);
        }

        // default quality
        $qualities['auto'] = new Movie_Variant('auto', 'auto', safe_get_value($files[0], 'url'));
        return $qualities;
    }

    /**
     * Quality label of the file: 'BDRIP 1080P'
     *
     * @param array $file
     * @return string
     */
    protected static function make_quality_name($file)
    {
        return strtoupper(trim(safe_get_value($file, 'quality', '') . ' ' . safe_get_value($file, 'hd_standard', '')));
    }

    /**
     * Names of the persons of the role matched by part of the role name.
     * Persons is the role keyed object: {"Режиссеры": [{"rus_name": "", "orig_name": ""}, ...], ...}
     *
     * @param array $film
     * @param string $role_stem
     * @return string
     */
    protected static function get_persons($film, $role_stem)
    {
        foreach (safe_get_value($film, 'persons', array()) as $role => $persons) {
            if (!is_array($persons) || strpos($role, $role_stem) === false) continue;

            $names = array();
            foreach ($persons as $person) {
                $name = safe_get_value($person, 'rus_name');
                if (empty($name)) {
                    $name = safe_get_value($person, 'orig_name');
                }

                if (!empty($name)) {
                    $names[] = $name;
                }
            }

            return implode(', ', $names);
        }

        return '';
    }

    /**
     * Stable GUID of the device for auth
     *
     * @return string
     */
    protected static function get_guid()
    {
        $hash = md5('torrenttv:' . get_serial_number());
        return sprintf('%s-%s-%s-%s-%s', substr($hash, 0, 8), substr($hash, 8, 4), substr($hash, 12, 4), substr($hash, 16, 4), substr($hash, 20, 12));
    }
}

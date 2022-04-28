<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://list.playtv.pro/tv_live-m3u8/%s-%s';
    const PLAYLIST_VOD_URL = 'http://list.playtv.pro/kino-full/%s-%s';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/live/(?<token>.+)/(?<id>.+)/.+\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8');

        $this->set_epg_param('first','epg_url', 'http://api.sramtv.com/get/?type=epg&ch={CHANNEL}');
        $this->set_epg_param('first','epg_root', '');
        $this->set_epg_param('second','epg_url', 'http://api.gazoni1.com/get/?type=epg&ch={CHANNEL}');
        $this->set_epg_param('second','epg_root', '');
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
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        if ($this->get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('/video.m3u8', '.ts', $url);
        }

        // hd_print("Stream url:  " . $url);

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

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $login, $password);
            case 'movie':
                return sprintf(self::PLAYLIST_VOD_URL, $login, $password);
        }

        return '';
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "utc=$archive_ts";
            // hd_print("Archive TS:  " . $archive_ts);
        }

        return $url;
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);

        // this account has special API to get account info
        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if ($force === false && !empty($login) && !empty($password)) {
            return true;
        }

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return false;
        }

        try {
            $url = sprintf('http://list.playtv.pro/api/dune-api5m.php?subscr=%s-%s',
                $plugin_cookies->login,
                $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            try {
                $url = sprintf('http://list.shara.tv/api/dune-api5m.php?subscr=%s-%s',
                    $plugin_cookies->login,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                return false;
            }
        }

        // stripe UTF8 BOM if exists
        $account_data = json_decode(ltrim($content, "\xEF\xBB\xBF"), true);
        return isset($account_data['status']) && $account_data['status'] === 'ok';
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = $this->GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
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
        $str_len = strlen($packages);
        if ($str_len === 0) {
            Control_Factory::add_label($defs, $title, 'Нет пакетов', 20);
            return;
        }

        if ($str_len < 30) {
            Control_Factory::add_label($defs, $title, $packages, 20);
            return;
        }

        $list = explode(', ', $packages);
        $emptyTitle = str_repeat(' ', strlen($title));
        $list_collected = array();
        $isFirstLabel = true;
        foreach($list as $item) {
            $list_collected[] = $item;
            $collected = implode(', ', $list_collected);
            if (strlen($collected) < 30) {
                continue;
            }

            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected);

            if ($isFirstLabel) {
                $isFirstLabel = false;
            }

            $list_collected = array();
        }

        if (count($list_collected) !== 0) {
            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
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
        $categories = HD::LoadAndStoreJson($url);
        if ($categories === false) {
            return;
        }

        HD::StoreContentToFile($categories, self::get_vod_cache_file());

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        foreach ($categories as $movie) {
            $category = (string)$movie->category;
            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Starnet_Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

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
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item->name, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateMovie($item);
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

        $arr = explode("_", $query_id);
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

        foreach ($jsonItems as $item) {
            if ($category_id === $item->category) {
                $movies[] = self::CreateMovie($item);
            }
        }

        hd_print("Movies read for query: $query_id - " . count($movies));
        return $movies;
    }

    /**
     * @param Object $movie_obj
     * @return Short_Movie
     */
    protected static function CreateMovie($movie_obj)
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

    protected static function get_vod_cache_file()
    {
        return DuneSystem::$properties['tmp_dir_path'] . "/playlist_vod.json";
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

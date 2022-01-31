<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://list.playtv.pro/tv_live-m3u8/%s-%s';
    const PLAYLIST_VOD_URL = 'http://list.playtv.pro/kino-full/%s-%s';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = true;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[BALANCE_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/live/(?<token>.+)/(?<id>.+)/.+\.m3u8$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
        static::$FEATURES[VOD_LAZY_LOAD] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = '';
        static::$EPG_PARSER_PARAMS['second']['epg_root'] = '';
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('/video.m3u8', '.ts', $url);
        }

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
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
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
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

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"), true);
        return isset($account_data['status']) && $account_data['status'] === 'ok';
    }

    public static function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = self::GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            ControlFactory::add_label($defs, 'Ошибка!', $text[0]);
            ControlFactory::add_label($defs, 'Описание:', $text[1]);
            return;
        }

        $title = 'Пакеты: ';

        ControlFactory::add_label($defs, 'Баланс:', $account_data['data']['money'] . ' руб.');
        ControlFactory::add_label($defs, 'Цена подписки:', $account_data['data']['money_need'] . ' руб.');
        $packages = $account_data['data']['abon'];
        $str_len = strlen($packages);
        if ($str_len === 0) {
            ControlFactory::add_label($defs, $title, 'Нет пакетов');
            return;
        }

        if ($str_len < 30) {
            ControlFactory::add_label($defs, $title, $packages);
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

            ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected);

            if ($isFirstLabel) {
                $isFirstLabel = false;
            }

            $list_collected = array();
        }

        if (count($list_collected) !== 0) {
            ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
        }
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        hd_print("Fetching EPG for ID: '$id'");
        switch($type)
        {
            case 'first':
                return sprintf('http://api.sramtv.com/get/?type=epg&ch=%s', $id);
            case 'second':
                return sprintf('http://api.gazoni1.com/get/?type=epg&ch=%s', $id);
        }

        return null;
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        $movie = new Movie($movie_id);
        $jsonItems = HD::parse_json_file(static::GET_VOD_TMP_STORAGE_PATH());
        foreach ($jsonItems as $item) {

            $id = -1;
            if (array_key_exists("id", $item)) {
                $id = $item["id"];
            } else if (array_key_exists("series_id", $item)) {
                $id = $item["series_id"] . "season";
            }

            if ($id !== (int)$movie_id) {
                continue;
            }

            $duration = "";
            if (array_key_exists("duration_secs", $item["info"])) {
                $duration = $item["info"]["duration_secs"] / 60;
            } else if (array_key_exists("episode_run_time", $item["info"])) {
                $duration = $item["info"]["episode_run_time"];
            }

            $genres = HD::ArrayToStr($item["info"]["genre"]);
            $country = HD::ArrayToStr($item["info"]["country"]);

            $movie->set_data(
                $item["name"],              // name,
                '',             // name_original,
                $item["info"]["plot"],      // description,
                $item["info"]["poster"],    // poster_url,
                $duration,                  // length_min,
                $item["info"]["year"],      // year,
                $item["info"]["director"],  // director_str,
                '',               // scenario_str,
                $item["info"]["cast"],      // actors_str,
                $genres,                    // genres_str,
                $item["info"]["rating"],    // rate_imdb,
                '',             // rate_kinopoisk,
                '',                // rate_mpaa,
                $country,                   // country,
                ''                   // budget
            );

            // case for serials
            if (array_key_exists("seasons", $item)) {
                foreach ($item["seasons"] as $season) {
                    $seasonNumber = $season["season"];
                    foreach ($season["episodes"] as $episode) {
                        $episodeCaption = "Сезон " . $seasonNumber . ":  Эпизод " . $episode['episode'];
                        $playback_url = $episode['video'];
                        if (preg_match('|^https?://(.+)/series/.+\.mp4(.+)$|', $playback_url, $matches)) {
                            $playback_url = str_replace(
                                array('{DOMAIN}', '{TOKEN}', '{ID}'),
                                array($matches[1], $matches[2], "vod-" . $episode['id']),
                                static::$FEATURES[MEDIA_URL_TEMPLATE_HLS]);
                        }
                        hd_print("movie playback_url: $playback_url");
                        $movie->add_series_data($episode['id'], $episodeCaption, $playback_url, true);
                    }
                }
            } else {
                $playback_url = str_replace("https://", "http://", $item["video"]);
                hd_print("movie playback_url: $playback_url");
                $movie->add_series_data($movie_id, $item['name'], $playback_url, true);
            }

            break;
        }

        return $movie;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $url = static::GetPlaylistUrl('movie', $plugin_cookies);
        $categories = HD::LoadAndStoreJson($url, true, self::GET_VOD_TMP_STORAGE_PATH());
        if ($categories === false)
        {
            return;
        }

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        foreach ($categories as $movie) {
            if (!in_array($movie["category"], $categoriesFound)) {
                $categoriesFound[] = $movie["category"];
                $cat = new StarnetVodCategory((string)$movie["category"], (string)$movie["category"]);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        hd_print("Categories read: " . count($category_list));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");
        $movies = array();
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item["name"], 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateMovie($item);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());

        $arr = explode("_", $query_id);
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

        foreach ($jsonItems as $item) {
            if ($category_id === $item["category"]) {
                $movies[] = self::CreateMovie($item);
            }
        }

        hd_print("Movies read: " . count($movies));
        return $movies;
    }

    /**
     * @throws Exception
     */
    protected static function CreateMovie($mov_array)
    {
        $id = -1;
        if (array_key_exists("id", $mov_array)) {
            $id = $mov_array["id"];
        } else if (array_key_exists("series_id", $mov_array)) {
            $id = $mov_array["series_id"] . "season";
        }

        $info_arr = $mov_array["info"];
        $genres = HD::ArrayToStr($info_arr["genre"]);
        $country = HD::ArrayToStr($info_arr["country"]);
        $movie = new ShortMovie($id, $mov_array["name"], $info_arr["poster"]);
        $movie->info = $mov_array["name"] . "|Год: " . $info_arr["year"] . "|Страна: $country|Жанр: $genres|Рейтинг: " . $info_arr["rating"];

        return  $movie;
    }

    public static function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        static::$movie_counter[$key] += $val;
    }
}

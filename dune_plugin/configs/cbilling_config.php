<?php
require_once 'default_config.php';

class CbillingPluginConfig extends DefaultConfig
{
    const API_HOST = 'http://protected-api.com';

    // vod
    const MOVIE_URL_TEMPLATE = 'http://%s%s?token=%s';

    const PLAYLIST_TV_URL = 'http://247on.cc/playlist/%s_otp_dev%s.m3u8';
    const MEDIA_URL_TEMPLATE_HLS2 = 'http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = true;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS', 'hls2' => 'HLS2', 'mpeg' => 'MPEG-TS');
        static::$FEATURES[DEVICE_OPTIONS] = array('1' => '1', '2' => '2', '3' => '3');
        static::$FEATURES[BALANCE_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>[^/]+)/s/(?<token>[^/]+)/?(?<id>.+)\.m3u8$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8';
        static::$FEATURES[VOD_LAZY_LOAD] = true;
        static::$FEATURES[EXTINF_VOD_PATTERN] = '^#EXTINF.+genres="([^"]*)"\s+rating="([^"]*)"\s+year="([^"]*)"\s+country="([^"]*)"\s+director="([^"]*)".*group-title="([^"]*)"\s*,\s*(.*)$|';

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = '';
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
        $ext_params = $channel->get_ext_params();
        $domain = explode(':', $ext_params['subdomain']);

        if (self::get_format($plugin_cookies) !== 'hls') {
            // http://s01.iptvx.tv/pervyj-hd/video.m3u8?token=8264fb5785dc128d5d64a681a94ba78f
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($domain[0], $channel->get_channel_id(), $ext_params['token']),
                self::MEDIA_URL_TEMPLATE_HLS2);
        }

        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                // http://s01.iptvx.tv:8090/s/8264fb5785dc128d5d64a681a94ba78f/pervyj-hd.m3u8
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = static::UpdateArchiveUrlParams($url, $archive_ts);
                }
                break;
            case 'hls2':
                if ((int)$archive_ts > 0) {
                    // http://s01.iptvx.tv/pervyj-hd/archive-{START}-10800.m3u8?token=8264fb5785dc128d5d64a681a94ba78f
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', 'video-' . $archive_ts . '-10800.m3u8', $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', 'archive-' . $archive_ts . '-10800.ts', $url);
                } else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
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
        if (!parent::GetAccountInfo($plugin_cookies, &$account_data, $force)) {
            return false;
        }

        $plugin_cookies->subdomain_local = $account_data['subdomain'];
        $plugin_cookies->token = $account_data['token'];

        // this account has special API to get account info
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if ($force === false && !empty($password)) {
            return true;
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $headers[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
            $content = HD::http_get_document(self::API_HOST . '/auth/info', $headers);
        } catch (Exception $ex) {
            return false;
        }

        // stripe UTF8 BOM if exists
        $account_data = json_decode(ltrim($content, "\xEF\xBB\xBF"), true);
        return true;
    }

    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = $this->GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            ControlFactory::add_label($defs, 'Ошибка!', $text[0], -10);
            ControlFactory::add_label($defs, 'Описание:', $text[1], -10);
            return;
        }

        ControlFactory::add_label($defs, 'Пакеты: ', empty($account_data['data']['package']) ? 'Нет пакетов' : $account_data['data']['package'], -10);
        ControlFactory::add_label($defs, 'Дата окончания', $account_data['data']['end_date'], -10);
        ControlFactory::add_label($defs, 'Устройств', $account_data['data']['devices_num'], -10);
        ControlFactory::add_label($defs, 'Сервер', $account_data['data']['server'], 20);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $password, isset($plugin_cookies->device_number) ? $plugin_cookies->device_number : '1');
            case 'movie':
                return self::API_HOST . '/genres';
        }

        return '';
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        $epg_date = gmdate(static::$EPG_PARSER_PARAMS[$type]['date_format'], $day_start_ts);
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('%s/epg/%s/?date=%s', self::API_HOST, $id, $epg_date); // epg_id date(Y-m-d)
        }

        return null;
    }

    /**
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        $movie = new Movie($movie_id);
        $json = HD::LoadAndStoreJson(self::API_HOST . "/video/$movie_id", false);
        if ($json === false) {
            return $movie;
        }

        $movieData = $json->data;

        $genresArray = array();
        foreach ($movieData->genres as $genre) {
            $genresArray[] = $genre->title;
        }

        $movie->set_data(
            $movieData->name,// caption,
            $movieData->original_name,// caption_original,
            $movieData->description,// description,
            $movieData->poster,// poster_url,
            $movieData->time,// length,
            $movieData->year,// year,
            $movieData->director,// director,
            '',// scenario,
            $movieData->actors,// actors,
            implode(", ", $genresArray),// $xml->genres,
            $movieData->rating,// rate_imdb,
            '',// rate_kinopoisk,
            $movieData->age,// rate_mpaa,
            $movieData->country,// country,
            ''// budget
        );

        $domain = explode(':', $plugin_cookies->subdomain_local);

        if (isset($movieData->seasons)) {
            foreach ($movieData->seasons as $season) {
                $seasonNumber = $season->number;
                foreach ($season->series as $episode) {
                    $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $episode->files[0]->url, $plugin_cookies->token);
                    hd_print("episode playback_url: $playback_url");
                    $episode_caption = "Сезон $seasonNumber| Серия $episode->number $episode->name";
                    $movie->add_series_data($episode->id, $episode_caption, $playback_url, true);
                }
            }
        } else {
            $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $movieData->files[0]->url, $plugin_cookies->token);
            hd_print("movie playback_url: $playback_url");
            $movie->add_series_data($movie_id, $movieData->name, $playback_url, true);
        }

        return $movie;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        //hd_print("fetch_vod_categories");
        $categories = HD::LoadAndStoreJson(self::API_HOST, false /*, "/tmp/run/vc.json" */);
        if ($categories === false) {
            return;
        }

        $category_list = array();
        $category_index = array();

        // all movies
        $category = new StarnetVodCategory('all', 'Все фильмы');
        $category_list[] = $category;
        $category_index[$category->get_id()] = $category;

        foreach ($categories->data as $node) {
            $id = (string)$node->id;
            $category = new StarnetVodCategory($id, (string)$node->name);

            // fetch genres for category
            $genres = HD::LoadAndStoreJson(self::API_HOST . "/cat/$id/genres", false);
            if ($genres === false) {
                continue;
            }

            $gen_arr = array();
            foreach ($genres->data as $genre) {
                $gen_arr[] = new StarnetVodCategory((string)$genre->id, (string)$genre->title, $category);
            }

            $category->set_sub_categories($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }

        hd_print("Categories read: " . count($category_list));
    }

    /**
     * @throws Exception
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        //hd_print("getSearchList");
        $url = self::API_HOST . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . $this->get_next_page($keyword);
        $searchRes = HD::LoadAndStoreJson($url, false /*, "/tmp/run/sl.json"*/);
        return $searchRes === false ? array() : $this->CollectSearchResult($searchRes);
    }

    /**
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        hd_print("getVideoList: $query_id");
        $val = $this->get_next_page($query_id);

        if ($query_id === 'all') {
            $url = "/filter/new?page=$val";
        } else {
            $arr = explode("_", $query_id);
            if ($arr === false) {
                $genre_id = $query_id;
            } else {
                $genre_id = $arr[1];
            }

            $url = "/genres/$genre_id?page=$val";
        }

        $categories = HD::LoadAndStoreJson(self::API_HOST . $url, false/*, "/tmp/run/$keyword.json"*/);
        return $categories === false ? array() : $this->CollectSearchResult($categories);
    }

    /**
     * @throws Exception
     */
    protected function CollectSearchResult($json)
    {
        //hd_print("CollectSearchResult");
        $movies = array();

        foreach ($json->data as $entry) {
            $genresArray = array();
            if (isset($entry->genres)) {
                foreach ($entry->genres as $genre) {
                    $genresArray[] = $genre->title;
                }
            }
            if (isset($entry->name)) {
                $movie = new ShortMovie($entry->id, $entry->name, $entry->poster);
                $genre_str = implode(", ", $genresArray);
                $movie->info = "$entry->name|Год: $entry->year|Страна: $entry->country|Жанр: $genre_str|Рейтинг: $entry->rating";
                $movies[] = $movie;
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    public static function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, static::$movie_counter)) {
            static::$movie_counter[$key] = 0;
        }

        static::$movie_counter[$key] += $val;
    }

    public static function get_device($plugin_cookies)
    {
        return isset($plugin_cookies->device_number) ? $plugin_cookies->device_number : '1';
    }
}

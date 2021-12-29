<?php
require_once 'default_config.php';

class AntifrizPluginConfig extends DefaultConfig
{
    const VOD_URL = 'http://api.iptvx.tv';
    const MOVIE_URL_TEMPLATE = 'http://%s%s?token=%s';

    // supported features
    public static $ACCOUNT_TYPE = 'PIN';
    public static $MPEG_TS_SUPPORTED = true;
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // tv
    protected static $PLAYLIST_TV_URL = 'http://antifriz.tv/playlist/%s.m3u8';
    protected static $PLAYLIST_VOD_URL = 'http://antifriz.tv/smartup/%s.m3u';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/s/(?<token>.+)/(?<id>.+)/.*$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/s/{TOKEN}/{ID}/video.m3u8';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'http://{DOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id

    // vod
    public static $EXTINF_VOD_PATTERN = '^#EXTINF.+genres="([^"]*)"\s+rating="([^"]*)"\s+year="([^"]*)"\s+country="([^"]*)"\s+director="([^"]*)".*group-title="([^"]*)"\s*,\s*(.*)$|';

    protected static $lazy_load_vod = true;

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
        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hls archive url completely different, make it from scratch
                    $domain = explode(':', $ext_params['subdomain']);
                    $url = str_replace(
                        array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}'),
                        array($domain[0], $channel->get_channel_id(), $ext_params['token'], $archive_ts),
                        self::$MEDIA_URL_TEMPLATE_ARCHIVE_HLS);
                }
                break;
            case 'mpeg':
                // mpeg url also different against hls, make it from scratch
                $url = ((int)$archive_ts > 0) ? self::$MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : self::$MEDIA_URL_TEMPLATE_MPEG;
                $domain = explode(':', $ext_params['subdomain']);
                $url = str_replace(
                    array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}'),
                    array($domain[0], $channel->get_channel_id(), $ext_params['token'], $archive_ts),
                    $url);
                $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
                break;
            default:
                hd_print("unknown url format");
        }

        // hd_print("Stream url:  " . $url);
        // hd_print("Domain:      " . $subdomain);
        // hd_print("Token:       " . $ext_params['token']);
        // hd_print("Archive TS:  " . $archive_ts);

        return HD::make_ts($url);
    }

    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        if (parent::GetAccountInfo($plugin_cookies, &$account_data, $force)) {
            $plugin_cookies->subdomain_local = $account_data['subdomain'];
            $plugin_cookies->ott_key_local = $account_data['token'];
            return true;
        }

        return false;
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::$PLAYLIST_TV_URL, $password);
            case 'movie':
                return sprintf(self::$PLAYLIST_VOD_URL, $password);
        }

        return '';
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        $movie = new Movie($movie_id);
        $json = HD::LoadAndStoreJson(self::VOD_URL . "/video/$movie_id", false);
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
                    $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $episode->files[0]->url, $plugin_cookies->ott_key_local);
                    hd_print("episode playback_url: $playback_url");
                    $episode_caption = "Сезон $seasonNumber| Серия $episode->number $episode->name";
                    $movie->add_series_data($episode->id, $episode_caption, $playback_url, true);
                }
            }
        } else {
            $playback_url = sprintf(self::MOVIE_URL_TEMPLATE, $domain[0], $movieData->files[0]->url, $plugin_cookies->ott_key_local);
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
        $categories = HD::LoadAndStoreJson(self::VOD_URL, false /*, "/tmp/run/vc.json" */);
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
            $genres = HD::LoadAndStoreJson(self::VOD_URL . "/cat/$id/genres", false);
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
    public static function getSearchList($keyword, $plugin_cookies)
    {
        //hd_print("getSearchList");
        $url = self::VOD_URL . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . static::get_next_page($keyword);
        $searchRes = HD::LoadAndStoreJson($url, false /*, "/tmp/run/sl.json"*/);
        return $searchRes === false ? array() : self::CollectSearchResult($searchRes);
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($idx, $plugin_cookies)
    {
        hd_print("getVideoList: $idx");
        $val = static::get_next_page($idx);

        if ($idx === 'all') {
            $url = "/filter/new?page=$val";
        } else {
            $arr = explode("_", $idx);
            if ($arr === false) {
                $genre_id = $idx;
            } else {
                $genre_id = $arr[1];
            }

            $url = "/genres/$genre_id?page=$val";
        }

        $categories = HD::LoadAndStoreJson(self::VOD_URL . $url, false/*, "/tmp/run/$keyword.json"*/);
        return $categories === false ? array() : self::CollectSearchResult($categories);
    }

    /**
     * @throws Exception
     */
    protected static function CollectSearchResult($json)
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
                $movie = new ShortMovie((string)$entry->id, (string)$entry->name, (string)$entry->poster);
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
}

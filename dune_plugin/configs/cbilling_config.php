<?php
require_once 'default_config.php';

class CbillingPluginConfig extends DefaultConfig
{
    // supported features
    const VOD_URL = 'http://api.iptvx.tv';
    const MOVIE_URL_TEMPLATE = 'http://%s%s?token=%s';
    public static $ACCOUNT_TYPE = 'PIN';
    public static $HLS2_SUPPORTED = true;
    public static $MPEG_TS_SUPPORTED = true;
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // tv
    protected static $PLAYLIST_TV_URL = 'http://cbilling.pw/playlist/%s_otp_dev1.m3u8';
    protected static $PLAYLIST_VOD_URL = 'http://api.iptvx.tv/';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>[^/]+)/s/(?<token>[^/]+)/?(?<id>.+)\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8';
    public static $MEDIA_URL_TEMPLATE_HLS2 = 'http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/cbilling/epg/%s.json'; // epg1_id
    protected static $EPG2_URL_TEMPLATE = 'http://api.iptvx.tv/epg/%s?date=%s'; // epg2_id

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
        $domain = explode(':', $ext_params['subdomain']);

        if (self::get_format($plugin_cookies) !== 'hls') {
            // http://s01.iptvx.tv/pervyj-hd/video.m3u8?token=8264fb5785dc128d5d64a681a94ba78f
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($domain[0], $channel->get_channel_id(), $ext_params['token']),
                self::$MEDIA_URL_TEMPLATE_HLS2);
        }

        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                // http://s01.iptvx.tv:8090/s/8264fb5785dc128d5d64a681a94ba78f/pervyj-hd.m3u8
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = self::UpdateArchiveUrlParams($url, $archive_ts);
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
                $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

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
/*
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
            $auth['code'] = $password;
            $post_body = json_encode($auth);
            $content = HD::http_post_document('http://api.iptvx.tv/auth', $post_body);
        } catch (Exception $ex) {
            return false;
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"), true);
        return isset($account_data['package_info']) && !empty($account_data['package_info']);
*/
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
                return self::$PLAYLIST_VOD_URL;
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

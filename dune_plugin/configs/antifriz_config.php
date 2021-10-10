<?php
require_once 'default_config.php';

class AntifrizPluginConfig extends DefaultConfig
{
    const VOD_URL = 'http://api.iptvx.tv';
    const EXTINF_VOD_PATTERN = '^#EXTINF.+genres="([^"]*)"\s+rating="([^"]*)"\s+year="([^"]*)"\s+country="([^"]*)"\s+director="([^"]*)".*group-title="([^"]*)"\s*,\s*(.*)$|';

    // info
    public static $PLUGIN_NAME = 'AntiFriz TV';
    public static $PLUGIN_SHORT_NAME = 'antifriz';
    public static $PLUGIN_VERSION = '1.0.2';
    public static $PLUGIN_DATE = '23.09.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://antifriz.tv/playlist/%s.m3u8';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/s/(?<token>.+)/(?<id>.+)/.*$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'http://{SUBDOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'http://{SUBDOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}';
    public static $CHANNELS_LIST = 'antifriz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://antifriz.tv/smartup/%s.m3u';
    public static $MOVIE_URL_TEMPLATE = 'http://%s%s?token=%s';

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $ext_params = $channel->get_ext_params();
        $domain = explode(':', $ext_params['subdomain']);
        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        $url = $channel->get_streaming_url();
        switch ($format) {
            case 'hls':
                if (intval($archive_ts) <= 0) break;

                $url = self::$MEDIA_URL_TEMPLATE_ARCHIVE_HLS;
                $url = str_replace('{SUBDOMAIN}', $domain[0], $url);
                $url = str_replace('{ID}', $ext_params['id'], $url);
                $url = str_replace('{TOKEN}', $ext_params['token'], $url);
                $url = str_replace('{START}', $archive_ts, $url);
                break;
            case 'mpeg':
                $url = (intval($archive_ts) > 0) ? self::$MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : self::$MEDIA_URL_TEMPLATE_MPEG;
                $url = str_replace('{SUBDOMAIN}', $domain[0], $url);
                $url = str_replace('{ID}', $ext_params['id'], $url);
                $url = str_replace('{TOKEN}', $ext_params['token'], $url);
                $url = str_replace('{START}', $archive_ts, $url);
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        // hd_print("Stream url:  " . $url);
        // hd_print("Domain:      " . $subdomain);
        // hd_print("Token:       " . $ext_params['token']);
        // hd_print("Archive TS:  " . $archive_ts);

        return static::make_ts($url);
    }

    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        return parent::GetPlaylistStreamInfo($plugin_cookies);
    }

    /**
     * Update url by provider additional parameters
     * @param $channel_id
     * @param $plugin_cookies
     * @param $ext_params
     * @return string
     */
    public static function UpdateStreamUri($channel_id, $plugin_cookies, $ext_params)
    {
        $url = str_replace('{SUBDOMAIN}', $ext_params['subdomain'], static::$MEDIA_URL_TEMPLATE_HLS);
        $url = str_replace('{ID}', $ext_params['id'], $url);
        $url = str_replace('{TOKEN}', $ext_params['token'], $url);
        return static::make_ts($url);
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        $movie = new Movie($movie_id);
        $json = static::LoadAndStoreJson(self::VOD_URL . "/video/$movie_id", false);
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
                    $playback_url = sprintf(self::$MOVIE_URL_TEMPLATE, $domain[0], $episode->files[0]->url, $plugin_cookies->ott_key_local);
                    //hd_print("episode playback_url: $playback_url");
                    $episode_caption = "Сезон $seasonNumber| Серия $episode->number $episode->name";
                    $movie->add_series_data($episode->id, $episode_caption, $playback_url, true);
                }
            }
        } else {
            $playback_url = sprintf(self::$MOVIE_URL_TEMPLATE, $domain[0], $movieData->files[0]->url, $plugin_cookies->ott_key_local);
            //hd_print("movie playback_url: $playback_url");
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
        $categories = static::LoadAndStoreJson(self::VOD_URL, false /*, "/tmp/run/vc.json" */);
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
            $id = strval($node->id);
            $category = new StarnetVodCategory($id, strval($node->name));

            // fetch genres for category
            $genres = static::LoadAndStoreJson(self::VOD_URL . "/cat/$id/genres", false);
            if ($genres === false) continue;

            $gen_arr = array();
            foreach ($genres->data as $genre) {
                $gen_arr[] = new StarnetVodCategory(strval($genre->id), strval($genre->title), $category);
            }

            $category->set_sub_categories($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }
    }

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        //hd_print("getSearchList");
        $url = self::VOD_URL . "/filter/by_name?name=" . urlencode($keyword) . "&page=" . static::get_next_page($keyword);
        $searchRes = static::LoadAndStoreJson($url, false /*, "/tmp/run/sl.json"*/);
        return $searchRes === false ? array() : self::CollectSearchResult($searchRes);
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($idx, $plugin_cookies)
    {
        //hd_print("getVideoList: $keyword");
        $val = static::get_next_page($idx);

        if ($idx == 'all') {
            $url = "/filter/new?page=$val";
        } else {
            $arr = explode("_", $idx);
            if ($arr === false)
                $genre_id = $idx;
            else
                $genre_id = $arr[1];

            $url = "/genres/$genre_id?page=$val";
        }

        $categories = static::LoadAndStoreJson(self::VOD_URL . $url, false/*, "/tmp/run/$keyword.json"*/);
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
                $movie = new ShortMovie(strval($entry->id), strval($entry->name), strval($entry->poster));
                $genre_str = implode(", ", $genresArray);
                $movie->info = "$entry->name|Год: $entry->year|Страна: $entry->country|Жанр: $genre_str|Рейтинг: $entry->rating";
                $movies[] = $movie;
            }
        }

        return $movies;
    }
}

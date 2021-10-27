<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://list.playtv.pro/api/dune-api5m.php?subscr=%s-%s';
    const ACCOUNT_INFO_URL2 = 'http://list.shara.tv/api/dune-api5m.php?subscr=%s-%s';
    const SERIES_VOD_PATTERN = '|^https?://(.+)/series/.+\.mp4(.+)$|';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://list.playtv.pro/tv_live-m3u8/%s-%s';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/live/(?<token>.+)/(?<id>.+)/.+\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    public static $CHANNELS_LIST = 'sharaclub_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYY-MM-DD)
    protected static $EPG2_URL_TEMPLATE = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYY-MM-DD)

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://list.playtv.pro/kino-full/%s-%s';

    public function __construct()
    {
        parent::__construct();
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
    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();

        if ((int)$archive_ts > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        if ($format === 'mpeg') {
            $url = str_replace('/video.m3u8', '.ts', $url);
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    /**
     * Get information from the account
     * @param $plugin_cookies
     * @param &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        // this account has special API to get account info
        if (empty($plugin_cookies->login) && empty($plugin_cookies->password)) {
            return false;
        }

        try {
            $url = sprintf(static::ACCOUNT_INFO_URL1,
                $plugin_cookies->login,
                $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            try {
                $url = sprintf(static::ACCOUNT_INFO_URL2,
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

    /**
     * Update url by provider additional parameters
     * @param $channel_id
     * @param $plugin_cookies
     * @param $ext_params
     * @return string
     */
    public static function UpdateStreamUri($channel_id, $plugin_cookies, $ext_params)
    {
        $url = str_replace(
            array('{ID}', '{SUBDOMAIN}', '{TOKEN}'),
            array($channel_id, $ext_params['subdomain'], $ext_params['token']),
            static::$MEDIA_URL_TEMPLATE_HLS);
        return static::make_ts($url);
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

            if ($id != $movie_id) continue;

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
                        if (preg_match(self::SERIES_VOD_PATTERN, $playback_url, $matches)) {
                            $playback_url = str_replace(
                                array('{SUBDOMAIN}', '{TOKEN}', '{ID}'),
                                array($matches[1], $matches[2], "vod-" . $episode['id']),
                                self::$MEDIA_URL_TEMPLATE_HLS);
                        }
                        $movie->add_series_data($episode['id'], $episodeCaption, $playback_url, true);
                    }
                }
            } else {
                $playback_url = str_replace("https://", "http://", $item["video"]);
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
        $url = sprintf(self::$MOVIE_LIST_URL_TEMPLATE, $plugin_cookies->login, $plugin_cookies->password);
        $categories = static::LoadAndStoreJson($url, true, static::GET_VOD_TMP_STORAGE_PATH());
        if ($categories === false)
        {
            return;
        }

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        foreach ($categories as $movie) {
            if (in_array($movie["category"], $categoriesFound, false)) continue;

            $categoriesFound[] = $movie["category"];
            $cat = new StarnetVodCategory((string)$movie["category"], (string)$movie["category"]);
            $category_list[] = $cat;
            $category_index[$cat->get_id()] = $cat;
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        $movies = array();
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item["name"], 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateMovie($item);
            }
        }

        return $movies;
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($idx, $plugin_cookies)
    {
        $movies = array();
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());

        $arr = explode("_", $idx);
        if ($arr === false) {
            $category_id = $idx;
        } else {
            $category_id = $arr[0];
        }

        foreach ($jsonItems as $item) {
            if ($category_id === $item["category"]) {
                $movies[] = self::CreateMovie($item);
            }
        }

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
        $movie = new ShortMovie((string)$id, (string)$mov_array["name"], (string)$info_arr["poster"]);
        $movie->info = $mov_array["name"] . "|Год: " . $info_arr["year"] . "|Страна: $country|Жанр: $genres|Рейтинг: " . $info_arr["rating"];

        return  $movie;
    }
}

<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://list.playtv.pro/api/dune-api5m.php?subscr=%s-%s';
    const ACCOUNT_INFO_URL2 = 'http://list.shara.tv/api/dune-api5m.php?subscr=%s-%s';
    const SERIES_VOD_PATTERN = '/^https?:\/\/([\w\.]+)\/series\/.+\.mp4([\w\.]+)$/';
    const VOD_PLAYLIST_NAME = 'vod_playlist.json';

    // info
    public static $PLUGIN_NAME = 'Sharaclub TV';
    public static $PLUGIN_SHORT_NAME = 'sharaclub';
    public static $PLUGIN_VERSION = '1.0.1';
    public static $PLUGIN_DATE = '12.09.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://list.playtv.pro/tv_live-m3u8/%s-%s';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/live/(?<token>.+)/.+/.+\.m3u8$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}.ts';
    public static $CHANNELS_LIST = 'sharaclub_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://list.playtv.pro/kino-full/%s-%s';

    /**
     * @return string
     */
    public static function GET_VOD_TMP_STORAGE_PATH()
    {
        return static::GET_TMP_STORAGE_PATH(self::VOD_PLAYLIST_NAME);
    }

    public static function GetAccountStatus($plugin_cookies)
    {
        // this account has special API to get account info
        if (empty($plugin_cookies->login) && empty($plugin_cookies->password))
            return false;

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

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"));
        if (isset($account_data->status) && $account_data->status == 'ok') {
            // hd_print("account ok");
            return $account_data;
        }

        return false;
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        $movie = new Movie($movie_id);
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());
        foreach ($jsonItems as $item) {

            $id = -1;
            if (array_key_exists("id", $item))
                $id = $item["id"];
            else if (array_key_exists("series_id", $item))
                $id = $item["series_id"] . "season";

            if ($id !== $movie_id) continue;

            $duration = "";
            if (array_key_exists("duration_secs", $item["info"]))
                $duration = $item["info"]["duration_secs"] / 60;
            else if (array_key_exists("episode_run_time", $item["info"]))
                $duration = $item["info"]["episode_run_time"];

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
    public static function getSearchList($keyword)
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
    public static function getVideoList($category_id, $genre_id)
    {
        $movies = array();
        $jsonItems = HD::parse_json_file(self::GET_VOD_TMP_STORAGE_PATH());
        foreach ($jsonItems as $item) {
            if ($category_id == $item["category"]) {
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
        if (array_key_exists("id", $mov_array))
            $id = $mov_array["id"];
        else if (array_key_exists("series_id", $mov_array))
            $id = $mov_array["series_id"] . "season";

        $movie = new ShortMovie(strval($id), strval($mov_array["name"]), strval($mov_array["info"]["poster"]));

        $genres = HD::ArrayToStr($mov_array["info"]["genre"]);
        $country = HD::ArrayToStr($mov_array["info"]["country"]);
        $movie->info = $mov_array["name"] . "|Год: " . $mov_array["info"]["year"] . "|Страна: " . $country . "|Жанр: " . $genres . "|IMDB: " . $mov_array["info"]["rating"];

        return $movie;
    }
}

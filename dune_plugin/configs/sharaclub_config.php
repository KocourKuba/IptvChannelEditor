﻿<?php
require_once 'default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    // local parameters
    const ACCOUNT_INFO_URL = 'http://%s/api/dune-api5m.php?subscr=%s-%s';
    const ACCOUNT_PLAYLIST_URL = 'http://%s/tv_live-m3u8/%s-%s';
    const ACCOUNT_PRIMARY_DOMAIN = 'list.playtv.pro';
    const ACCOUNT_SECONDARY_DOMAIN = 'list.shara.tv';
    const STREAM_URL_PATTERN = '/^https?:\/\/(.+)\/live\/(.+)\/.+\/.*\.m3u8$/';
    const SERIES_VOD_PATTERN = '/^https?:\/\/([\w\.]+)\/series\/.+\.mp4([\w\.]+)$/';
    const VOD_PLAYLIST_NAME = 'vod_playlist.json';

    // info
    public static $PLUGIN_NAME = 'Sharaclub TV';
    public static $PLUGIN_SHORT_NAME = 'sharaclub';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '01.09.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $USE_LOGIN_PASS = true;

    // tv
    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    public static $CHANNELS_LIST = 'sharaclub_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://list.playtv.pro/kino-full/%s-%s';

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';

        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format) {
            case 'hls':
                break;
            case 'mpeg':
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url = str_replace('/video.m3u8', '.ts', $url);
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
        }

        return $url;
    }

    /**
     * @return string
     */
    public static function GET_VOD_TMP_STORAGE_PATH()
    {
        return static::GET_TMP_STORAGE_PATH(self::VOD_PLAYLIST_NAME);
    }

    public static function GetAccountStatus($plugin_cookies)
    {
        if (empty($plugin_cookies->login) && empty($plugin_cookies->password))
            return false;

        try {
            $url = sprintf(self::ACCOUNT_INFO_URL,
                self::ACCOUNT_PRIMARY_DOMAIN,
                $plugin_cookies->login,
                $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            try {
                $url = sprintf(self::ACCOUNT_INFO_URL,
                    self::ACCOUNT_SECONDARY_DOMAIN,
                    $plugin_cookies->login,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                return false;
            }
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"));
        if (isset($account_data->status) && $account_data->status == 'ok') {
            hd_print("account ok");
            return $account_data;
        }

        return false;
    }

    public static function GetAccessInfo($plugin_cookies)
    {
        hd_print("Collect information from sharaclub account");
        $found = false;
        if (!empty($plugin_cookies->login) && !empty($plugin_cookies->password)) {
            try {
                $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                    self::ACCOUNT_PRIMARY_DOMAIN,
                    $plugin_cookies->login,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                try {
                    $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                        self::ACCOUNT_SECONDARY_DOMAIN,
                        $plugin_cookies->login,
                        $plugin_cookies->password);
                    $content = HD::http_get_document($url);
                } catch (Exception $ex) {
                    return false;
                }
            }

            $tmp_file = DefaultConfig::GET_TMP_STORAGE_PATH('playlist.m3u8');
            file_put_contents($tmp_file, $content);
            $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            for ($i = 0; $i < count($lines); ++$i) {
                if (preg_match(self::STREAM_URL_PATTERN, $lines[$i], $matches)) {
                    $plugin_cookies->subdomain_local = $matches[1];
                    $plugin_cookies->ott_key_local = $matches[2];
                    $found = true;
                    break;
                }
            }
            unlink($tmp_file);
        }

        return $found;
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
                                self::$MEDIA_URL_TEMPLATE);
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

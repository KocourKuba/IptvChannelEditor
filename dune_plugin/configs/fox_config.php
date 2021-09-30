<?php
require_once 'default_config.php';

class FoxPluginConfig extends DefaultConfig
{
    const SERIES_VOD_PATTERN = '|^https?://.+/vod/(.+)\.mp4/video\.m3u8\?token=.+$|';
    const EXTINF_VOD_PATTERN = '|^#EXTINF:.+group-title="(.*)".+tvg-logo="(.*)"\s*,\s*(.*)$|';
    const EXTINF_TV_PATTERN  = '|^#EXTINF:.+CUID="(\d+)".+$|';

    // info
    public static $PLUGIN_NAME = 'Fox TV';
    public static $PLUGIN_SHORT_NAME = 'fox';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '1.10.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = false;
    public static $VOD_FAVORITES_SUPPORTED = false;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';
    public static $STREAM_URL_PATTERN = '|^https?://([^/]+)/([^/]+)(?:/.+\.m3u8){0,1}$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{TOKEN}/index.m3u8';
    public static $CHANNELS_LIST = 'fox_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/fox-tv/epg/%s.json'; // epg_id date(YYYYMMDD)

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/vod.m3u';

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $format = static::get_format($plugin_cookies);
        $url = $channel->get_streaming_url();

        switch ($format) {
            case 'hls':
                break;
            case 'mpeg':
                $url = str_replace('/index.m3u8', '', $url);
                $url = str_replace('/video.m3u8', '', $url);
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        if (intval($archive_ts) > 0) {
            $url .= '?utc={START}&lutc={NOW}';
        }

        // hd_print("Stream type: " . $format);
        // hd_print("Stream url:  " . $url);
        // hd_print("Channel ID:  " . $id);
        // hd_print("Archive TS:  " . $archive_ts);

        $url = str_replace('{START}', $archive_ts, $url);
        $url = str_replace('{NOW}', strval(time()), $url);

        return static::make_ts($url);
    }

    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $tmp_file = static::GET_TMP_STORAGE_PATH();
        try {
            $content = self::FetchTemplatedUrl(static::$ACCOUNT_TYPE, static::$ACCOUNT_PLAYLIST_URL1, $plugin_cookies);
            file_put_contents($tmp_file, $content);

            $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            for ($i = 0; $i < count($lines); ++$i) {
                if (preg_match(self::EXTINF_TV_PATTERN, $lines[$i], $m_id)) {
                    $pl_entries[$m_id[1]] = $lines[$i + 1];
                }
            }
        } catch (Exception $ex) {
            hd_print("Failed to fetch provider playlist");
        }

        hd_print("Read Playlist entries: " . count($pl_entries));
        return $pl_entries;
    }

    public static function GetAccountInfo($plugin_cookies)
    {
        hd_print("Collect information from account " . static::$PLUGIN_NAME);
        try {
            $content = self::FetchTemplatedUrl(static::$ACCOUNT_TYPE, static::$ACCOUNT_PLAYLIST_URL1, $plugin_cookies);
        } catch (Exception $ex) {
            hd_print("Failed to fetch provider playlist");
            return false;
        }

        $tmp_file = static::GET_TMP_STORAGE_PATH();
        file_put_contents($tmp_file, $content);
        $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        for ($i = 0; $i < count($lines); ++$i) {
            if (preg_match(static::$STREAM_URL_PATTERN, $lines[$i], $matches)) {
                return true;
            }
        }

        return false;
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        //hd_print("Movie ID: $movie_id");
        $movie = new Movie($movie_id);
        $m3u_lines = file(static::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if($i != $movie_id || !preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $logo = $matches[2];
            $caption = $matches[3];

            $url = $m3u_lines[$i + 1];
            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $caption,// $xml->caption,
                '',// $xml->caption_original,
                '',// $xml->description,
                $logo,// $xml->poster_url,
                '',// $xml->length,
                '',// $xml->year,
                '',// $xml->director,
                '',// $xml->scenario,
                '',// $xml->actors,
                '',// $xml->genres,
                '',// $xml->rate_imdb,
                '',// $xml->rate_kinopoisk,
                '',// $xml->rate_mpaa,
                '',// $xml->country,
                ''// $xml->budget
            );

            $movie->add_series_data($movie_id, $caption, $url, true);
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
        try {
            $doc = HD::http_get_document($url);
            if (empty($doc)) {
                hd_print("empty playlist or not valid token");
                return;
            }

            file_put_contents(self::GET_VOD_TMP_STORAGE_PATH(), $doc);
        } catch (Exception $ex) {
            hd_print("Unable to load movie categories: " . $ex->getMessage());
            return;
        }

        $m3u_lines = file(self::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $category = $matches[1];
            if(empty($category))
                $category = 'Без категории';

            if (!in_array($category, $categoriesFound)) {
                array_push($categoriesFound, $category);
                $cat = new StarnetVodCategory($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }
    }

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword)
    {
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = file(static::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $logo = $matches[2];
            $caption = $matches[3];

            $search  = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new ShortMovie(strval($i), $caption, $logo);
            }
        }

        return $movies;
    }

    /**
     * @throws Exception
     */
    public static function getVideoList($idx)
    {
        $movies = array();
        $m3u_lines = file(static::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $category = $matches[1];
            $logo = $matches[2];
            $caption = $matches[3];
            if(empty($category))
                $category = 'Без категории';

            $arr = explode("_", $idx);
            if ($arr === false)
                $category_id = $idx;
            else
                $category_id = $arr[0];

            if ($category_id == $category) {
                $movies[] = new ShortMovie(strval($i), $caption, $logo);
            }
        }

        return $movies;
    }
}

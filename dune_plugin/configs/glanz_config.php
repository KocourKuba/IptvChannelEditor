﻿<?php
/** @noinspection DuplicatedCode */
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    const SERIES_VOD_PATTERN = '|^https?://.+/vod/(.+)\.mp4/video\.m3u8\?token=.+$|';
    const EXTINF_VOD_PATTERN = '|^#EXTINF.+group-title="(?<category>.*)".+tvg-logo="(?<logo>.*)"\s*,\s*(?<title>.*)$|';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $CHANNELS_LIST = 'glanz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=vod';

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

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

        $format = static::get_format($plugin_cookies);
        // hd_print("Stream type: " . $format);
        switch ($format) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "video-$archive_ts-10800.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "archive-$archive_ts-10800.ts", $url);
                }
                else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
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
        if ($ext_params === null || !isset($ext_params['subdomain'], $ext_params['token'])) {
            hd_print("UpdateStreamUri: parameters for $channel_id not defined!");
            return '';
        }

        $url = str_replace(
            array(
                '{SUBDOMAIN}',
                '{ID}',
                '{TOKEN}',
                '{LOGIN}',
                '{PASSWORD}',
                '{INT_ID}',
                '{HOST}'
            ),
            array(
                $ext_params['subdomain'],
                $channel_id,
                $ext_params['token'],
                $ext_params['login'],
                $ext_params['password'],
                $ext_params['int_id'],
                $ext_params['host']
            ),
            static::$MEDIA_URL_TEMPLATE_HLS);

        return static::make_ts($url);
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        //hd_print("Movie ID: $movie_id");
        $movie = new Movie($movie_id);
        $m3u_lines = file(static::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($m3u_lines as $i => $iValue) {
            if ($i !== (int)$movie_id || !preg_match(self::EXTINF_VOD_PATTERN, $iValue, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

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
     * @noinspection DuplicatedCode
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

        foreach ($m3u_lines as $line) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new StarnetVodCategory($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }
    }

    /**
     * @throws Exception
     */
    public static function getSearchList($keyword, $plugin_cookies)
    {
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $iValue, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

            $search = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new ShortMovie((string)$i, $caption, $logo);
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
        $m3u_lines = file(static::GET_VOD_TMP_STORAGE_PATH(), FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($m3u_lines as $i => $iValue) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $iValue, $matches)) {
                continue;
            }

            $category = $matches['category'];
            $logo = $matches['logo'];
            $caption = $matches['title'];
            if(empty($category)) {
                $category = 'Без категории';
            }

            $arr = explode("_", $idx);
            if ($arr === false) {
                $category_id = $idx;
            } else {
                $category_id = $arr[0];
            }

            if ($category_id === $category) {
                $movies[] = new ShortMovie((string)$i, $caption, $logo);
            }
        }

        return $movies;
    }
}

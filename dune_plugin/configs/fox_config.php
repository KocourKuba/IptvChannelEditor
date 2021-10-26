<?php
require_once 'default_config.php';

class FoxPluginConfig extends DefaultConfig
{
    const SERIES_VOD_PATTERN = '|^https?://.+/vod/(.+)\.mp4/video\.m3u8\?token=.+$|';
    const EXTINF_VOD_PATTERN = '|^#EXTINF:.+tvg-logo="([^"]+)".+group-title="([^"]+)".*,\s*(.*)$|';
    const EXTINF_TV_PATTERN  = '|^#EXTINF:.+CUID="(?<id>\d+)".+$|';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $MPEG_TS_SUPPORTED = false;
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/?(?<hls>.+\.m3u8){0,1}$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{TOKEN}/index.m3u8';
    public static $CHANNELS_LIST = 'fox_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/fox-tv/epg/%s.json'; // epg_id

    // vod
    public static $MOVIE_LIST_URL_TEMPLATE = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';

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
        // entire url replaced in UpdateStreamUri, subdomain not subst
        $ext_params = $channel->get_ext_params();
        $url = $channel->get_streaming_url();

        if (intval($archive_ts) > 0) {
            $now_ts = strval(time());
            $url .= "?utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
        }

        if (!isset($ext_params['hls'])) {
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $m3u_lines = static::FetchTvM3U($plugin_cookies);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (preg_match(self::EXTINF_TV_PATTERN, $m3u_lines[$i], $m_id)) {
                if (preg_match(self::$M3U_STREAM_URL_PATTERN, $m3u_lines[$i + 1], $matches)) {
                    $pl_entries[$m_id['id']] = $matches;
                }
            }
        }

        hd_print("Read Playlist entries: " . count($pl_entries));
        return $pl_entries;
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
        // fox does not have variable parameters, only token. Replace entire url
        return static::make_ts($ext_params[0]);
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        //hd_print("Movie ID: $movie_id");
        $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
        $movie = new Movie($movie_id);

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $match)) continue;

            $logo = $match[1];
            $caption = explode('/', $match[3]);
            $title = $caption[0];
            $title_orig = $caption[1];
            $url = static::make_ts($m3u_lines[$i + 1]) . "|||dune_params|||buffering_ms:$buf_time";

            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $title,// $xml->caption,
                $title_orig,// $xml->caption_original,
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

            $movie->add_series_data($movie_id, $title, $url, true);
            break;
        }

        return $movie;
    }

    /**
     * @throws Exception
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $category = $matches[2];
            if (empty($category))
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
    public static function getSearchList($keyword, $plugin_cookies)
    {
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $logo = $matches[1];
            $caption = $matches[3];

            $search = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new ShortMovie(strval($i), $caption, $logo);
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
        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        for ($i = 0; $i < count($m3u_lines); ++$i) {
            if (!preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $matches)) continue;

            $logo = $matches[1];
            $category = $matches[2];
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

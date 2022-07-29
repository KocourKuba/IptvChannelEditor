<?php
require_once 'default_config.php';

class GlanzPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    const PLAYLIST_VOD_URL = 'http://api.ottg.tv/playlist/vod?login=%s&password=%s';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(SECONDARY_EPG, true);
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/{ID}/video-{START}-10800.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{ID}/archive-{START}-10800.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(EXTINF_VOD_PATTERN, '|^#EXTINF.+group-title="(?<category>.*)".+tvg-logo="(?<logo>.*)"\s*,\s*(?<title>.*)$|');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('first','epg_url', 'http://epg.iptvx.one/api/id/{CHANNEL}.json');
        $this->set_epg_param('first','epg_root', 'ch_programme');
        $this->set_epg_param('first','epg_start', 'start');
        $this->set_epg_param('first','epg_end', '');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_time_format', 'd-m-Y H:i');
        $this->set_epg_param('first','epg_timezone', 'Europe/Moscow');

        $this->set_epg_param('second','epg_url', 'http://technic.cf/epg-iptvxone/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('second','epg_root', 'data');
        $this->set_epg_param('second','epg_start', 'begin');
        $this->set_epg_param('second','epg_end', 'end');
        $this->set_epg_param('second','epg_title', 'title');
        $this->set_epg_param('second','epg_description', 'description');
        $this->set_epg_param('second','epg_date_format', 'Y.m.d');
        $this->set_epg_param('second','epg_use_mapper', true);
        $this->set_epg_param('second','epg_mapper_url', 'http://technic.cf/epg-iptvxone/channels');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $url = $channel->get_streaming_url();
        if (!empty($url)) {
            $url = static::UpdateArchiveUrlParams($url, $archive_ts);
        } else {
            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $url = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_HLS : MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $url = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array(
                    '{DOMAIN}',
                    '{ID}',
                    '{LOGIN}',
                    '{PASSWORD}',
                    '{TOKEN}',
                    '{INT_ID}',
                    '{HOST}',
                    '{START}'
                ),
                array(
                    $ext_params['subdomain'],
                    $channel->get_channel_id(),
                    $ext_params['login'],
                    $ext_params['password'],
                    $ext_params['token'],
                    $ext_params['int_id'],
                    $ext_params['host'],
                    $archive_ts
                ),
                $url);
        }

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $login, $password);
            case 'movie':
                return sprintf(self::PLAYLIST_VOD_URL, $login, $password);
        }

        return '';
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        hd_print("TryLoadMovie: $movie_id");
        $movie = new Movie($movie_id);
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());

        if ($jsonItems === false) {
            hd_print("TryLoadMovie: failed to load movie: $movie_id");
            return $movie;
        }

        foreach ($jsonItems as $item) {
            if (isset($item->id)) {
                $id = (string)$item->id;
            } else {
                $id = '-1';
            }

            if ($id !== $movie_id) {
                continue;
            }

            $genres = array();
            foreach ($item->genres as $genre) {
                if (!empty($genre->title)) {
                    $genres[] = $genre->title;
                }
            }
            $genres_str = implode(", ", $genres);

            $movie->set_data(
                $item->name,            // name,
                $item->o_name,          // name_original,
                $item->description,     // description,
                $item->cover,           // poster_url,
                '',           // length_min,
                $item->year,            // year,
                $item->director,        // director_str,
                '',           // scenario_str,
                $item->actors,          // actors_str,
                $genres_str,            // genres_str,
                '',            // rate_imdb,
                '',         // rate_kinopoisk,
                '',            // rate_mpaa,
                $item->country,         // country,
                ''               // budget
            );

            $playback_url = str_replace("https://", "http://", $item->url);
            hd_print("movie playback_url: $playback_url");
            $movie->add_series_data($movie_id, $item->name, '', $playback_url);
            break;
        }

        return $movie;
    }

    /**
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $url = $this->GetPlaylistUrl('movie', $plugin_cookies);
        $categories = HD::DownloadJson($url);
        if ($categories === false) {
            return;
        }

        HD::StoreContentToFile($categories, self::get_vod_cache_file());

        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        foreach ($categories as $movie) {
            $category = (string)$movie['category'];
            if (empty($category)) {
                $category = "Без категории";
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Starnet_Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        hd_print("Categories read: " . count($category_list));
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");
        $movies = array();
        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print("getSearchList: failed to load movies");
            return $movies;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));
        foreach ($jsonItems as $item) {
            $search  = utf8_encode(mb_strtolower($item->name, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = self::CreateShortMovie($item);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();

        $jsonItems = HD::parse_json_file(self::get_vod_cache_file());
        if ($jsonItems === false) {
            hd_print("getVideoList: failed to load movies");
            return $movies;
        }

        $arr = explode("_", $query_id);
        if ($arr === false) {
            $category_id = $query_id;
        } else {
            $category_id = $arr[0];
        }

        foreach ($jsonItems as $item) {
            $category = $item->category;
            if (empty($category)) {
                $category = "Без категории";
            }

            if ($category_id === $category) {
                $movies[] = self::CreateShortMovie($item);
            }
        }

        hd_print("Movies read for query: $query_id - " . count($movies));
        return $movies;
    }

    /**
     * @param Object $movie_obj
     * @return Short_Movie
     */
    protected static function CreateShortMovie($movie_obj)
    {
        if (isset($movie_obj->id)) {
            $id = (string)$movie_obj->id;
        } else {
            $id = '-1';
        }

        $genres = array();
        foreach ($movie_obj->genres as $genre) {
            if (!empty($genre->title)) {
                $genres[] = $genre->title;
            }
        }
        $genres_str = implode(", ", $genres);

        $movie = new Short_Movie($id, (string)$movie_obj->name, (string)$movie_obj->cover);
        $movie->info = "$movie_obj->name|Год: $movie_obj->year|Страна: $movie_obj->country|Жанр: $genres_str";

        return $movie;
    }

    protected static function get_vod_cache_file()
    {
        return get_temp_path("playlist_vod.json");
    }

    /**
     * @param string $key
     * @param int $val
     */
    public function add_movie_counter($key, $val)
    {
        // repeated count data
        if (!array_key_exists($key, $this->movie_counter)) {
            $this->movie_counter[$key] = 0;
        }

        $this->movie_counter[$key] += $val;
    }
}

﻿<?php
require_once 'lib/epg_manager.php';
require_once 'lib/tv/channel.php';

class Default_Config
{
    protected $PluginShortName;

    // features constants
    protected $FEATURES = array();
    protected $STREAMS = array();
    protected $EPG_PARSER_PARAMS = array();

    // page counter for some plugins
    protected $pages = array();
    protected $is_entered = false;
    protected $movie_counter = array();
    protected $filters = array();

    protected $embedded_account;

    public function __construct($short_name)
    {
        $this->PluginShortName = $short_name;

        $this->FEATURES[ACCOUNT_TYPE] = -1;
        $this->FEATURES[TV_FAVORITES_SUPPORTED] = true;
        $this->FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = false;
        $this->FEATURES[VOD_FAVORITES_SUPPORTED] = false;
        $this->FEATURES[VOD_QUALITY_SUPPORTED] = false;
        $this->FEATURES[VOD_FILTER_SUPPORTED] = false;
        $this->FEATURES[VOD_LAZY_LOAD] = false;
        $this->FEATURES[DEVICE_OPTIONS] = array();
        $this->FEATURES[BALANCE_SUPPORTED] = false;
        $this->FEATURES[SERVER_SUPPORTED] = false;
        $this->FEATURES[QUALITY_SUPPORTED] = false;
        $this->FEATURES[EXTINF_VOD_PATTERN] = '';
        $this->FEATURES[SQUARE_ICONS] = false;
        $this->FEATURES[USE_TOKEN_AS_ID] = false;
    }

    /**
     * load configuration
     * @return void
     */
    public function load_config()
    {
        $default_streams = array();
        $default_streams[CU_TYPE] = 'shift';
        $default_streams[URL_TEMPLATE] = '';
        $default_streams[URL_ARC_TEMPLATE] = '{CU_SUBST}={START}&lutc={NOW}';
        $default_streams[CU_SUBST] = 'utc';
        $this->set_streams(HLS, $default_streams);

        $default_streams[CU_TYPE] = 'flussonic';
        $default_streams[CU_SUBST] = 'archive';
        $default_streams[URL_ARC_TEMPLATE] = '';
        $this->set_streams(MPEG, $default_streams);

        $default_parser = array();
        $default_parser[EPG_PARSER] = 'json';
        $default_parser[EPG_URL] = '';
        $default_parser[EPG_ROOT] = 'epg_data';
        $default_parser[EPG_START] = 'time';
        $default_parser[EPG_END] = 'time_to';
        $default_parser[EPG_NAME] = 'name';
        $default_parser[EPG_DESC] = 'descr';
        $default_parser[EPG_DATE_FORMAT] = '';
        $default_parser[EPG_USE_DURATION] = false;
        $default_parser[EPG_USE_MAPPER] = false;
        $default_parser[EPG_MAPPER_URL] = '';
        $default_parser[EPG_TIME_FORMAT] = '';
        $default_parser[EPG_TIMEZONE] = 0;
        $default_parser[EPG_ID_MAPPER] = array();

        $this->set_epg_params(EPG_FIRST, $default_parser);
        $this->set_epg_params(EPG_SECOND, $default_parser);
    }

    /**
     * @return Object|null
     */
    public function get_embedded_account()
    {
        return $this->embedded_account;
    }

    /**
     * @param Object $val
     */
    public function set_embedded_account($val)
    {
        $this->embedded_account = $val;
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_feature($type)
    {
        return $this->FEATURES[$type];
    }

    /**
     * @param string $type
     * @param mixed $val
     */
    public function set_feature($type, $val)
    {
        $this->FEATURES[$type] = $val;
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_device($plugin_cookies)
    {
        return null;
    }

    public function set_device($device, $plugin_cookies)
    {
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_quality_opts($plugin_cookies)
    {
        return array();
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_quality_id($plugin_cookies)
    {
        return null;
    }

    public function set_quality_id($quality, $plugin_cookies)
    {
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_server_opts($plugin_cookies)
    {
        return array();
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_server_id($plugin_cookies)
    {
        return null;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function get_server($plugin_cookies)
    {
        return null;
    }

    /**
     * @param $plugin_cookies
     * @return null
     */
    public function set_server($plugin_cookies)
    {
        return null;
    }

    public function get_login($plugin_cookies)
    {
        return isset($this->embedded_account->login) ? $this->embedded_account->login : $plugin_cookies->login;
    }

    public function get_password($plugin_cookies)
    {
        return isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;
    }

    /**
     * @param $plugin_cookies
     * @param string &$used_list
     * @return array $all_channels
     */
    public function get_channel_list($plugin_cookies, &$used_list)
    {
        $used_list = isset($plugin_cookies->channels_list) ? (string)$plugin_cookies->channels_list : sprintf('%s_channel_list.xml', $this->PluginShortName);
        if (!isset($plugin_cookies->channels_list))
            $plugin_cookies->channels_list = $used_list;

        $channels_source = isset($plugin_cookies->channels_source) ? (int)$plugin_cookies->channels_source : 1;
        if (!isset($plugin_cookies->channels_source))
            $plugin_cookies->channels_source = $channels_source;

        hd_print("Channels list name: $used_list");
        hd_print("Channels source: $channels_source");

        switch ($channels_source) {
            case 1: // folder
                $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
                break;
            case 2: // url
                $channels_list_path = get_install_path();
                break;
            default:
                return array();
        }

        hd_print("Channels list search path: $channels_list_path");

        $all_channels = array();
        $list = glob($channels_list_path . '*.xml');
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename !== 'dune_plugin.xml') {
                hd_print("Found channels list: $filename");
                $all_channels[$filename] = $filename;
            }
        }

        if (empty($all_channels)) {
            hd_print("No channels list found in selected location: " . $channels_list_path);
            return $all_channels;
        }

        if (!in_array($used_list, $all_channels)) {
            $used_list = (string)reset($all_channels);
        }

        hd_print("Used channels list: $used_list");
        return $all_channels;
    }

    /**
     * @param Channel $a
     * @param Channel $b
     * @return int
     */
    public static function sort_channels_cb($a, $b)
    {
        // Sort by channel numbers.
        return strnatcasecmp($a->get_number(), $b->get_number());
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_streams($type)
    {
        return $this->STREAMS[$type];
    }

    /**
     * @param string $type
     * @param array $val
     */
    public function set_streams($type, $val)
    {
        $this->STREAMS[$type] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @return mixed
     */
    public function get_stream_param($type, $param)
    {
        return $this->STREAMS[$type][$param];
    }

    /**
     * @param string $type
     * @param string $param
     * @param mixed $val
     */
    public function set_stream_param($type, $param, $val)
    {
        $this->STREAMS[$type][$param] = $val;
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_epg_params($type)
    {
        return $this->EPG_PARSER_PARAMS[$type];
    }

    /**
     * @param string $type
     * @param array $val
     */
    public function set_epg_params($type, $val)
    {
        $this->EPG_PARSER_PARAMS[$type] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @param mixed $val
     */
    public function set_epg_param($type, $param, $val)
    {
        $this->EPG_PARSER_PARAMS[$type][$param] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @return mixed
     */
    public function get_epg_param($type, $param)
    {
        return $this->EPG_PARSER_PARAMS[$type][$param];
    }

    public function try_reset_pages()
    {
        if ($this->is_entered) {
            $this->is_entered = false;
            $this->pages = array();
        }
    }

    public function reset_movie_counter()
    {
        $this->is_entered = true;
        $this->movie_counter = array();
    }

    /**
     * @param mixed $key
     * @return integer
     */
    public function get_movie_counter($key)
    {
        if (!array_key_exists($key, $this->movie_counter)) {
            $this->movie_counter[$key] = 0;
        }

        return $this->movie_counter[$key];
    }

    /**
     * @param mixed $key
     * @param integer $val
     */
    public function add_movie_counter($key, $val)
    {
        // entire list available, counter is final
        $this->movie_counter[$key] = $val;
    }

    /**
     * @param string $idx
     * @param int $increment
     * @return int
     */
    public function get_next_page($idx, $increment = 1)
    {
        if (!array_key_exists($idx, $this->pages)) {
            $this->pages[$idx] = 0;
        }

        $this->pages[$idx] += $increment;

        return $this->pages[$idx];
    }

    /**
     * @param string $idx
     * @param int $value
     */
    public function set_next_page($idx, $value)
    {
        $this->pages[$idx] = $value;
    }

    /**
     * @param string $name
     * @return mixed|null
     */
    public function get_filter($name)
    {
        return isset($this->filters[$name]) ? $this->filters[$name] : null;
    }

    /**
     * @param array $filters
     */
    public function set_filters($filters)
    {
        $this->filters = $filters;
    }

    /**
     * @param array $defs
     * @param Starnet_Vod_Filter_Screen $parent
     * @param int $initial
     * @return bool
     */
    public function AddFilterUI(&$defs, $parent, $initial = -1)
    {
        return false;
    }

    /**
     * @param array $user_input
     * @return string
     */
    public function CompileSaveFilterItem($user_input)
    {
        return '';
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        Control_Factory::add_label($defs, 'Баланс:', 'Информация о балансе не поддерживается');
    }

    /**
     * Generate url from template with macros substitution
     * Make url ts wrapped
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $url = $channel->get_streaming_url();
        if (empty($url)) {
            $is_archive = (int)$archive_ts > 0;
            $stream_type = $this->get_format($plugin_cookies);
            switch ($this->get_stream_param($stream_type, CU_TYPE)) {
                case 'append':
                case 'shift':
                    $url = $this->get_stream_param($stream_type, URL_TEMPLATE);
                    if ($is_archive) {
                        $url .= (strrpos($url, '?', -1) === false) ? '?' : '&';
                        $url .= $this->get_stream_param($stream_type, URL_ARC_TEMPLATE);
                    }
                    $url = str_replace(array('{START}', '{NOW}'), array($archive_ts, time()), $url);
                    break;
                case 'flussonic':
                    $url = $this->get_stream_param($stream_type, $is_archive ? URL_ARC_TEMPLATE : URL_TEMPLATE);
                    $url = str_replace(array('{START}', '{DURATION}'), array($archive_ts, time()), $url);
                    break;
            }

            $url = str_replace(
                array('{ID}', '{CU_SUBST}'),
                array($channel->get_channel_id(), $this->get_stream_param($stream_type, CU_SUBST)),
                $url);

            $ext_params = $channel->get_ext_params();
            $replaces = array(
                M_SUBDOMAIN => '{SUBDOMAIN}',
                M_DOMAIN    => '{DOMAIN}',
                M_PORT      => '{PORT}',
                M_LOGIN     => '{LOGIN}',
                M_PASSWORD  => '{PASSWORD}',
                M_TOKEN     => '{TOKEN}',
                M_INT_ID    => '{INT_ID}',
                M_HOST      => '{HOST}',
                M_QUALITY   => '{QUALITY}',
            );

            foreach ($replaces as $key => $value)
            {
                if (isset($ext_params[$key]) && !empty($ext_params[$key])) {
                    $url = str_replace($value, $ext_params[$key], $url);
                }
            }
        }

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * Update url by channel ID (for correct hash calculation of url)
     * @param string $channel_id
     * @param array $ext_params
     * @return string
     */
    public function GetUrlHash($channel_id, $ext_params)
    {
        if ($this->get_stream_param(HLS, URL_TEMPLATE) !== '')
            $url = $this->get_stream_param(HLS, URL_TEMPLATE);
        else if ($this->get_stream_param(MPEG, URL_TEMPLATE) !== '')
            $url = $this->get_stream_param(MPEG, URL_TEMPLATE);
        else {
            hd_print("No url template defined!");
            return 0;
        }
        if ($this->get_feature(USE_TOKEN_AS_ID))
            $url = str_replace('{TOKEN}', $ext_params[M_TOKEN], $url);
        else
            $url = str_replace('{ID}', $channel_id, $url);

        return hash("crc32", $url);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account");
        $m3u_lines = $this->FetchTvM3U($plugin_cookies, $force);
        foreach ($m3u_lines as $line) {
            if (preg_match($this->get_feature(URI_PARSE_TEMPLATE), $line, $matches)) {
                return $matches;
            }
        }

        return false;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (preg_match($this->get_feature(URI_PARSE_TEMPLATE), $line, $matches)) {
                $pl_entries[$matches[M_ID]] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
        }

        return $pl_entries;
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearPlaylistCache()
    {
        $tmp_file = get_temp_path("playlist_tv.m3u8");
        hd_print("Clear playlist cache: $tmp_file");
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearChannelsCache($plugin_cookies)
    {
        $tmp_file = get_temp_path($plugin_cookies->channels_list);
        hd_print("Clear channels cache: $tmp_file");
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

            $search = utf8_encode(mb_strtolower($caption, 'UTF-8'));
            if (strpos($search, $keyword) !== false) {
                $movies[] = new Short_Movie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @param string $params
     * @param $plugin_cookies
     * @return array
     */
    public function getFilterList($params, $plugin_cookies)
    {
        //hd_print("getFilterList: $params");
        return array();
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     */
    public function getVideoList($query_id, $plugin_cookies)
    {
        $movies = array();
        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            $logo = $matches['logo'];
            $caption = $matches['title'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            $arr = explode("_", $query_id);
            $category_id = ($arr === false) ? $query_id : $arr[0];
            if ($category_id === $category) {
                $movies[] = new Short_Movie((string)$i, $caption, $logo);
            }
        }

        hd_print("Movies read: " . count($movies));
        return $movies;
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        return null;
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $now_ts = time();
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    /**
     * @param string $url
     * @param $plugin_cookies
     * @return string
     */
    protected function UpdateMpegTsBuffering($url, $plugin_cookies)
    {
        if ($this->get_format($plugin_cookies) === MPEG) {
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        return HD::make_ts($url);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_format($plugin_cookies)
    {
        return isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
    }

    /**
     * @param $plugin_cookies
     * @param bool $force
     * @return array|false
     */
    protected function FetchTvM3U($plugin_cookies, $force = false)
    {
        $tmp_file = get_temp_path("playlist_tv.m3u8");
        if ($force !== false || !file_exists($tmp_file)) {
            try {
                $url = $this->GetPlaylistUrl('tv1', $plugin_cookies);
                //hd_print("tv1 m3u8 playlist: " . $url);
                if (empty($url)) {
                    hd_print("Tv1 playlist not defined");
                    throw new Exception('Tv1 playlist not defined');
                }
                file_put_contents($tmp_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                try {
                    $url = $this->GetPlaylistUrl('tv2', $plugin_cookies);
                    //hd_print("tv2 m3u8 playlist: " . $url);
                    if (empty($url)) {
                        throw new Exception("Tv2 playlist not defined");
                    }

                    file_put_contents($tmp_file, HD::http_get_document($url));
                } catch (Exception $ex) {
                    hd_print("Unable to load secondary tv playlist: " . $ex->getMessage());
                    return array();
                }
            }
        }

        return file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    }

    /**
     * @param $plugin_cookies
     * @param bool $force
     * @return array|false
     */
    public function FetchVodM3U($plugin_cookies, $force = false)
    {
        $m3u_file = get_temp_path("playlist_vod.m3u8");

        if ($force !== false || !file_exists($m3u_file)) {
            try {
                $url = $this->GetVodlistUrl($plugin_cookies);
                if (empty($url)) {
                    hd_print('Vod playlist not defined');
                    throw new Exception('Vod playlist not defined');
                }

                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print("Unable to load movie playlist: " . $ex->getMessage());
                return array();
            }
        }

        return file($m3u_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    }

    /**
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetch_vod_categories($plugin_cookies, &$category_list, &$category_index)
    {
        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (!preg_match($this->FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $category = $matches['category'];
            if (empty($category)) {
                $category = 'Без категории';
            }

            if (!in_array($category, $categoriesFound)) {
                $categoriesFound[] = $category;
                $cat = new Vod_Category($category, $category);
                $category_list[] = $cat;
                $category_index[$cat->get_id()] = $cat;
            }
        }

        hd_print("Categories read: " . count($category_list));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");
        if ($type === 'tv1') {
            $url = $this->get_feature(PLAYLIST_TEMPLATE);
        } else {
            $url = $this->get_feature(PLAYLIST_TEMPLATE2);
        }

        $login = $this->get_login($plugin_cookies);
        if (strpos($url, '{LOGIN}') !== false) {
            if (empty($login))
                hd_print("Login not set");
            else
                $url = str_replace('{LOGIN}', $login, $url);
        }

        $password = $this->get_password($plugin_cookies);
        if (strpos($url, '{PASSWORD}') !== false) {
            if (empty($password))
                hd_print("Password not set");
            else
                $url = str_replace('{PASSWORD}', $password, $url);
        }

        $this->ensure_token_loaded($plugin_cookies);
        if (strpos($url, '{TOKEN}') !== false) {
            if (empty($plugin_cookies->token))
                hd_print("Token not set");
            else
                $url = str_replace('{TOKEN}', $plugin_cookies->token, $url);
        }

        if (strpos($url, '{SUBDOMAIN}') !== false) {
            $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain, $url);
        }

        if (strpos($url, '{SERVER_ID}') !== false) {
            $url = str_replace('{SERVER_ID}', $this->get_server_id($plugin_cookies), $url);
        }

        if (strpos($url, '{SERVER}') !== false) {
            $url = str_replace('{SERVER}', $this->get_server($plugin_cookies), $url);
        }

        if (strpos($url, '{QUALITY}') !== false) {
            $url = str_replace('{QUALITY}', $this->get_quality_id($plugin_cookies), $url);
        }

        if (strpos($url, '{DEVICE}') !== false) {
            $url = str_replace('{DEVICE}', $this->get_device($plugin_cookies), $url);
        }

        return $url;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        return '';
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected function ensure_token_loaded(&$plugin_cookies)
    {
        return true;
    }
}

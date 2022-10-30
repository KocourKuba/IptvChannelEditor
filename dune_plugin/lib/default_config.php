<?php
require_once 'dynamic_config.php';
require_once 'epg_manager.php';
require_once 'tv/channel.php';
require_once 'm3u/M3uParser.php';

class default_config extends dynamic_config
{
    // page counter for some plugins
    protected $pages = array();
    protected $is_entered = false;
    protected $movie_counter = array();
    protected $filters = array();
    protected $embedded_account;
    protected $account_data = array();

    /**
     * @var Entry
     */
    protected $tv_m3u_entries;
    /**
     * @var Entry
     */
    protected $vod_m3u_entries;

    public function load_embedded_account()
    {
        $acc_file = get_install_path('account.dat');
        if (file_exists($acc_file)) {
            $data = file_get_contents($acc_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            if ($data !== false) {
                hd_print("account data: $data");
                $account = json_decode(base64_decode(substr($data, 5)));
                if ($account !== false) {
                    $this->embedded_account = $account;
                    //foreach ($this->embedded_account as $key => $item) hd_print("Embedded info: $key => $item");
                }
            }
        }
    }

    /**
     * @param $plugin_cookies
     * @return Entry[]
     */
    public function get_vod_m3u_entries($plugin_cookies)
    {
        if (empty($this->vod_m3u_entries)) {
            $m3u_lines = $this->FetchVodM3U($plugin_cookies);
            $parser = new M3uParser();
            $this->vod_m3u_entries = $parser->parseArray($m3u_lines);
            hd_print("Total entries loaded from VOD m3u file:" . count($this->vod_m3u_entries));
        }

        return $this->vod_m3u_entries;
    }

    /**
     * @param $plugin_cookies
     * @return Entry[]
     */
    public function get_tv_m3u_entries($plugin_cookies)
    {
        if (empty($this->tv_m3u_entries)) {
            $m3u_lines = $this->FetchTvM3U($plugin_cookies);
            $parser = new M3uParser();
            $this->tv_m3u_entries = $parser->parseArray($m3u_lines);
            hd_print("Total entries loaded from playlist m3u file:" . count($this->tv_m3u_entries));
        }

        return $this->tv_m3u_entries;
    }

    /**
     * @return Object|null
     */
    public function get_embedded_account()
    {
        return $this->embedded_account;
    }

    /**
     * @param Object|null $val
     */
    public function set_embedded_account($val)
    {
        $this->embedded_account = $val;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_login($plugin_cookies)
    {
        return isset($this->embedded_account->login) ? $this->embedded_account->login : (isset($plugin_cookies->login) ? $plugin_cookies->login : '');
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_password($plugin_cookies)
    {
        return isset($this->embedded_account->password) ? $this->embedded_account->password : (isset($plugin_cookies->password) ? $plugin_cookies->password : '');
    }

    /**
     * @param $plugin_cookies
     * @param string &$used_list
     * @return array $all_channels
     */
    public function get_channel_list($plugin_cookies, &$used_list)
    {
        if (empty($plugin_cookies->channels_list)) {
            $plugin_cookies->channels_list = sprintf('%s_channel_list.xml', $this->PluginShortName);
        }
        $used_list = $plugin_cookies->channels_list;

        if (!isset($plugin_cookies->channels_source)) {
            $plugin_cookies->channels_source = 1;
        }

        switch ($plugin_cookies->channels_source) {
            case 1: // folder
                hd_print("Channels source: folder");
                $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
                break;
            case 2: // url
                hd_print("Channels source: url");
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
        $now = time();
        $is_archive = (int)$archive_ts > 0;
        $stream_type = $this->get_format($plugin_cookies);
        $ext_params = $channel->get_ext_params();
        $ext_params[CHANNEL_ID] = $channel->get_channel_id();
        $ext_params[CU_START] = $archive_ts;
        $ext_params[CU_NOW] = $now;
        $ext_params[CU_OFFSET] = $now - $archive_ts;
        $ext_params[CU_SUBST] = $this->get_stream_param($stream_type, CU_SUBST);
        $ext_params[CU_DURATION] = $this->get_stream_param($stream_type, CU_DURATION);

        $replaces = array(
            CHANNEL_ID  => '{ID}',
            CU_START    => '{START}',
            CU_NOW      => '{NOW}',
            CU_DURATION => '{DURATION}',
            CU_OFFSET   => '{OFFSET}',
            CU_SUBST    => '{CU_SUBST}',
            M_SUBDOMAIN => '{SUBDOMAIN}',
            M_DOMAIN    => '{DOMAIN}',
            M_PORT      => '{PORT}',
            M_LOGIN     => '{LOGIN}',
            M_PASSWORD  => '{PASSWORD}',
            M_TOKEN     => '{TOKEN}',
            M_INT_ID    => '{INT_ID}',
            M_HOST      => '{HOST}',
            M_QUALITY   => '{QUALITY_ID}',
        );

        $channel_custom_url = $channel->get_custom_url();
        $channel_custom_arc_url = $channel->get_custom_archive_template();
        if (empty($channel_custom_url)) {
            // url template, live or archive
            $live_url = $this->get_stream_param($stream_type, URL_TEMPLATE);

            if (empty($channel_custom_arc_url)) {
                // global url archive template
                $archive_url = $this->get_stream_param($stream_type, URL_ARC_TEMPLATE);
            } else {
                // custom archive url template
                $archive_url = $channel_custom_arc_url;
            }
        } else {
            // custom url
            $live_url = $channel_custom_url;

            if (empty($channel_custom_arc_url)) {
                // global custom url archive template
                $archive_url = $this->get_stream_param($stream_type, URL_CUSTOM_ARC_TEMPLATE);
            } else {
                // custom url archive or template
                $archive_url = $channel_custom_arc_url;
            }
        }

        if ($is_archive) {
            // replace macros to live url
            $play_template_url = str_replace('{LIVE_URL}', $live_url, $archive_url);
            $custom_stream_type = $channel->get_custom_archive_url_type();
        } else {
            $play_template_url = $live_url;
            $custom_stream_type = $channel->get_custom_url_type();
        }

        // hd_print("play template: $play_template_url");
        // foreach($ext_params as $key => $value) {hd_print("ext_params: key: $key, value: $value");}

        // replace all macros
        foreach ($replaces as $key => $value)
        {
            if (isset($ext_params[$key]) && !empty($ext_params[$key])) {
                $play_template_url = str_replace($value, $ext_params[$key], $play_template_url);
            }
        }

        //hd_print("Stream url:  $url");

        $url = $this->UpdateMpegTsBuffering($play_template_url, $plugin_cookies, $custom_stream_type);

        return HD::make_ts($url);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account: $force");

        $parse_pattern = $this->get_feature(URI_PARSE_PATTERN);
        if (!empty($parse_pattern))
            $parse_pattern = "/$parse_pattern/";

        foreach ($this->get_tv_m3u_entries($plugin_cookies) as $entry) {
            if (preg_match($parse_pattern, $entry->getPath(), $matches)) {
                $this->account_data = $matches;
                return $this->account_data;
            }
        }

        return false;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public function GetPlaylistStreamsInfo($plugin_cookies)
    {
        hd_print("Get playlist information");
        $pl_entries = array();
        $parse_pattern = $this->get_feature(URI_PARSE_PATTERN);
        if (empty($parse_pattern)) {
            hd_print('Empty parse pattern. Unable to parse playlist');
            $this->ClearPlaylistCache();
            return $pl_entries;
        }
        $parse_pattern = "/$parse_pattern/";

        $tag_id = $this->get_feature(TAG_ID_MATCH);
        foreach ($this->get_tv_m3u_entries($plugin_cookies) as $entry) {
            if (!empty($tag_id)) {
                // special case for name, otherwise take ID from selected tag
                $id = ($tag_id === 'name') ? $entry->getTitle() : $entry->findAttribute($tag_id);
                if (empty($id)) {
                    hd_print("Unable to map ID by $tag_id for entry with url: " . $entry->getPath());
                    continue;
                }
            }

            // http://some_domain/some_token/index.m3u8
            if (preg_match($parse_pattern, $entry->getPath(), $matches)) {
                $id = !empty($tag_id) ? $id : $matches['id'];
                $pl_entries[$id] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('No channels mapped. Empty provider playlist or no channels mapped to playlist entries');
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
        $tmp_file = get_temp_path($this->PluginShortName . "_playlist_tv.m3u8");
        hd_print("Clear playlist cache: $tmp_file");
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearVodCache()
    {
        $tmp_file = get_temp_path($this->PluginShortName . "_playlist_vod.m3u8");
        hd_print("Clear VOD cache: $tmp_file");
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
        if (!$this->get_feature(VOD_M3U)) {
            return $movies;
        }

        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        foreach ($this->get_vod_m3u_entries($plugin_cookies) as $i => $entry) {
            $title = $entry->getTitle();
            $search_in = utf8_encode(mb_strtolower($title, 'UTF-8'));
            if (strpos($search_in, $keyword) === false) continue;

            $movies[] = new Short_Movie((string)$i, $entry->getEntryTitle(), $entry->findAttribute('tvg-logo'));
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
        hd_print("getVideoList: $query_id");
        $movies = array();

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        foreach ($this->get_vod_m3u_entries($plugin_cookies) as $i => $entry) {
            $category = $entry->getGroupTitle();
            if (empty($category)) {
                $category = 'Без категории';
            }

            if ($category_id === $category) {
                $movies[] = new Short_Movie((string)$i, $entry->getEntryTitle(), $entry->findAttribute('tvg-logo'));
            }
        }

        hd_print("Movies read: " . count($movies));
        return $movies;
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

        $vod_pattern = $this->get_feature(VOD_PARSE_PATTERN);
        if (!empty($vod_pattern))
            $vod_pattern = "/$vod_pattern/";

        $entries = $this->get_vod_m3u_entries($plugin_cookies);
        if (isset($entries[$movie_id])) {
            $entry = $entries[$movie_id];
            $logo = $entry->findAttribute('tvg-logo');
            $title = $entry->getTitle();
            $title_orig = '';
            $country = '';
            $year = '';

            if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match)) {
                $title = isset($match['title']) ? $match['title'] : $title;
                $title_orig = isset($match['title_orig']) ? $match['title_orig'] : $title_orig;
                $country = isset($match['country']) ? $match['country'] : $country;
                $year = isset($match['year']) ? $match['year'] : $year;
            }

            $url = $this->UpdateMpegTsBuffering($entry->getPath(), $plugin_cookies);

            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $title,// $xml->caption,
                $title_orig,// $xml->caption_original,
                '',// $xml->description,
                $logo,// $xml->poster_url,
                '',// $xml->length,
                $year,// $xml->year,
                '',// $xml->director,
                '',// $xml->scenario,
                '',// $xml->actors,
                '',// $xml->genres,
                '',// $xml->rate_imdb,
                '',// $xml->rate_kinopoisk,
                '',// $xml->rate_mpaa,
                $country,// $xml->country,
                ''// $xml->budget
            );

            $movie->add_series_data($movie_id, $title, '', $url);
            // hd_print("movie_id: $movie_id");
            // hd_print("title: $title");
            hd_print("movie url: $url");
        }

        return $movie;
    }

    /**
     * @param string $url
     * @param $plugin_cookies
     * @param int $custom_type
     * @return string
     */
    protected function UpdateMpegTsBuffering($url, $plugin_cookies, $custom_type = '')
    {
        $type = empty($custom_type) ? $this->get_format($plugin_cookies) : $custom_type;
        if ($type === MPEG) {
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
            $url .= "|||dune_params|||buffering_ms:$buf_time";
        }

        return $url;
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
        $tmp_file = get_temp_path($this->PluginShortName . "_playlist_tv.m3u8");
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
    protected function FetchVodM3U($plugin_cookies, $force = false)
    {
        $m3u_file = get_temp_path($this->PluginShortName . "_playlist_vod.m3u8");

        if ($force !== false || !file_exists($m3u_file)) {
            try {
                $url = $this->GetVodListUrl($plugin_cookies);
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
        hd_print("fetch_vod_categories");
        $category_list = array();
        $category_index = array();
        $categoriesFound = array();

        $vod_pattern = $this->get_feature(VOD_PARSE_PATTERN);
        if (!empty($vod_pattern))
            $vod_pattern = "/$vod_pattern/";

        foreach ($this->get_vod_m3u_entries($plugin_cookies) as $entry) {
            $category = $entry->getGroupTitle();
            if (empty($category)) {
                $category = 'Без категории';
            }

            $entry_title = $entry->getEntryTitle();
            if (empty($entry_title)) {
                $title = $entry->getTitle();
                if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match) && isset($match['title'])) {
                    $title = $match['title'];
                }
                $entry->setEntryTitle($title);
            }

            if (!in_array($category, $categoriesFound)) {
                hd_print("Found VOD category: $category");
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

        return $this->replace_subs_vars($url, $plugin_cookies);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        return $this->replace_subs_vars($this->get_feature(VOD_PLAYLIST_URL), $plugin_cookies);
    }

    /**
     * @param string $url
     * @param $plugin_cookies
     * @return string
     */
    protected function replace_subs_vars($url, $plugin_cookies)
    {
        if (!empty($url)) {
            if (strpos($url, '{LOGIN}') !== false) {
                $login = $this->get_login($plugin_cookies);
                if (empty($login))
                    hd_print("Login not set");
                else
                    $url = str_replace('{LOGIN}', $login, $url);
            }

            if (strpos($url, '{PASSWORD}') !== false) {
                $password = $this->get_password($plugin_cookies);
                if (empty($password))
                    hd_print("Password not set");
                else
                    $url = str_replace('{PASSWORD}', $password, $url);
            }

            if (strpos($url, '{TOKEN}') !== false) {
                $this->ensure_token_loaded($plugin_cookies);
                if (empty($plugin_cookies->token))
                    hd_print("Token not set");
                else
                    $url = str_replace('{TOKEN}', $plugin_cookies->token, $url);
            }

            if (strpos($url, '{SUBDOMAIN}') !== false) {
                $url = str_replace('{SUBDOMAIN}', $plugin_cookies->subdomain, $url);
            }

            if (strpos($url, '{SERVER}') !== false) {
                $url = str_replace('{SERVER}', $this->get_server_name($plugin_cookies), $url);
            }

            if (strpos($url, '{SERVER_ID}') !== false) {
                $url = str_replace('{SERVER_ID}', $this->get_server_id($plugin_cookies), $url);
            }

            if (strpos($url, '{QUALITY_ID}') !== false) {
                $url = str_replace('{QUALITY_ID}', $this->get_quality_id($plugin_cookies), $url);
            }

            if (strpos($url, '{DEVICE_ID}') !== false) {
                $url = str_replace('{DEVICE_ID}', $this->get_device_id($plugin_cookies), $url);
            }
        }
        return $url;
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

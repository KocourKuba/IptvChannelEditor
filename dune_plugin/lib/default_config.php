<?php

require_once 'default_dune_plugin.php';
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

    /**
     * @var array
     */
    protected $account_data;

    /**
     * @var Default_Dune_Plugin
     */
    protected $parent;
    /**
     * @var M3uParser
     */
    protected $m3u_parser;
    /**
     * @var Entry
     */
    protected $tv_m3u_entries;
    /**
     * @var array[]
     */
    protected $vod_m3u_indexes;

    public function __construct()
    {
        $this->m3u_parser = new M3uParser();
    }

    public function set_parent($parent)
    {
        $this->parent = $parent;
    }

    public function load_embedded_account()
    {
        $plugin_account = get_install_path('account.dat');
        $plugin_data = '';
        if (file_exists($plugin_account)) {
            $plugin_data = file_get_contents($plugin_account, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            hd_print("account data: $plugin_data");
            if ($plugin_data !== false) {
                $plugin_data = base64_decode(substr($plugin_data, 5));
            }
        }

        $backup_account = get_data_path('account.dat');
        $backup_data = '';
        if (file_exists($backup_account)) {
            $backup_data = file_get_contents($backup_account, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        }

        if (empty($plugin_data) && empty($backup_data))
            return;

        if (!empty($plugin_data)) {
            if ($plugin_data !== $backup_data) {
                hd_print("backup account data.");
                $backup_data = $plugin_data;
                file_put_contents($backup_account, $backup_data);
            }
        } else if (!empty($backup_data)) {
            hd_print("using backup account data.");
            $plugin_data = $backup_data;
        }

        $account = json_decode($plugin_data);
        if ($account !== false) {
            hd_print("account data loaded.");
            $this->embedded_account = $account;
            //foreach ($this->embedded_account as $key => $item) hd_print("Embedded info: $key => $item");
        }
    }

    /**
     * @return Entry[]
     */
    public function get_tv_m3u_entries()
    {
        if (empty($this->tv_m3u_entries)) {
            $this->tv_m3u_entries = $this->m3u_parser->parseInMemory();
            hd_print("Total entries loaded from playlist m3u file:" . count($this->tv_m3u_entries));
            HD::ShowMemoryUsage();
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
     * @return string
     */
    public function get_server_name($plugin_cookies)
    {
        $servers = $this->get_servers($plugin_cookies);
        return $servers[$this->get_server_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server_id($plugin_cookies)
    {
        $embedded_acc = $this->get_embedded_account();
        if (isset($embedded_acc, $embedded_acc->server_id)) {
            $server = $embedded_acc->server_id;
        }

        $servers = $this->get_servers($plugin_cookies);
        reset($servers);
        $first = key($servers);
        // first from cookies, second - embedded, last - top of list
        return isset($plugin_cookies->server, $servers[$plugin_cookies->server]) ? $plugin_cookies->server : (isset($server) ? $server : $first);
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
        $plugin_cookies->server = $server;
    }

    /**
     * @return string
     */
    public function get_device_name($plugin_cookies)
    {
        $devices = $this->get_devices($plugin_cookies);
        return $devices[$this->get_device_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_device_id($plugin_cookies)
    {
        $embedded_acc = $this->get_embedded_account();
        if (isset($embedded_acc, $embedded_acc->device_id)) {
            $plugin_cookies->device = $embedded_acc->device_id;
        }

        $devices = $this->get_devices($plugin_cookies);
        //reset($devices);
        $first = key($devices);
        return isset($plugin_cookies->device, $devices[$plugin_cookies->device]) ? $plugin_cookies->device : $first;
    }

    /**
     * @param $device
     * @param $plugin_cookies
     */
    public function set_device_id($device, $plugin_cookies)
    {
        $plugin_cookies->device = $device;
    }

    /**
     * @return string
     */
    public function get_quality_name($plugin_cookies)
    {
        $qualities = $this->get_qualities($plugin_cookies);
        return $qualities[$this->get_quality_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_quality_id($plugin_cookies)
    {
        $embedded_acc = $this->get_embedded_account();
        if (isset($embedded_acc, $embedded_acc->quality_id)) {
            $plugin_cookies->quality = $embedded_acc->quality_id;
        }

        $quality = $this->get_qualities($plugin_cookies);
        reset($quality);
        $first = key($quality);
        return isset($plugin_cookies->quality, $quality[$plugin_cookies->quality]) ? $plugin_cookies->quality : $first;
    }

    /**
     * @param $quality
     * @param $plugin_cookies
     */
    public function set_quality_id($quality, $plugin_cookies)
    {
        $plugin_cookies->quality = $quality;
    }

    /**
     * @return string
     */
    public function get_profile_name($plugin_cookies)
    {
        $profiles = $this->get_profiles($plugin_cookies);
        return $profiles[$this->get_profile_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return string|null
     */
    public function get_profile_id($plugin_cookies)
    {
        $embedded_acc = $this->get_embedded_account();
        if (isset($embedded_acc, $embedded_acc->profile_id)) {
            $plugin_cookies->profile = $embedded_acc->profile_id;
        }

        $profiles = $this->get_profiles($plugin_cookies);
        reset($profiles);
        $first = key($profiles);
        return isset($plugin_cookies->profile) && array_key_exists($plugin_cookies->profile, $profiles) ? $plugin_cookies->profile : $first;
    }

    /**
     * @param $profile
     * @param $plugin_cookies
     */
    public function set_profile_id($profile, $plugin_cookies)
    {
        $plugin_cookies->profile = $profile;
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
            case 3: // direct url
                hd_print("Channels source: direct url");
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

    public function get_vod_list_names($plugin_cookies, &$current_idx)
    {
        $vod_lists_array = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        $current_idx = isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
        $vod_lists = array();
        foreach ($vod_lists_array as $list) {
            $vod_lists[] = $list[Plugin_Constants::PLAYLIST_NAME];
        }
        return $vod_lists;
    }

    public function get_vod_list($plugin_cookies, &$current_idx)
    {
        $vod_lists_array = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        $current_idx = isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
        $vod_lists = array();
        foreach ($vod_lists_array as $list) {
            $vod_lists[$list[Plugin_Constants::PLAYLIST_NAME]] = $list[Plugin_Constants::URI_TEMPLATE];
        }
        return $vod_lists;
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
     * @param $tv
     * @param $plugin_cookies
     * @param bool $force
     * @return void
     */
    public function SetupM3uParser($tv, $plugin_cookies, $force = false)
    {
        if ($tv) {
            $playlist = $this->FetchTvM3U($plugin_cookies, $force);
        } else {
            $playlist = $this->FetchVodM3U($plugin_cookies, $force);
        }
        $this->m3u_parser->setupParser($playlist);
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
        $ext_params[Plugin_Constants::CHANNEL_ID] = $channel->get_channel_id();
        $ext_params[Stream_Params::CU_START] = $archive_ts;
        $ext_params[Stream_Params::CU_NOW] = $now;
        $ext_params[Stream_Params::CU_OFFSET] = $now - $archive_ts;
        $ext_params[Stream_Params::CU_STOP] = $archive_ts + $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);
        $ext_params[Stream_Params::CU_DURATION] = $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);
        $ext_params[Ext_Params::M_DEVICE_ID] = $this->get_device_id($plugin_cookies);
        $ext_params[Ext_Params::M_SERVER_ID] = $this->get_server_id($plugin_cookies);

        $replaces = array(
            Plugin_Constants::CHANNEL_ID => '{ID}',
            Stream_Params::CU_START      => '{START}',
            Stream_Params::CU_NOW        => '{NOW}',
            Stream_Params::CU_DURATION   => '{DURATION}',
            Stream_Params::CU_STOP       => '{STOP}',
            Stream_Params::CU_OFFSET     => '{OFFSET}',
            Ext_Params::M_SUBDOMAIN      => '{SUBDOMAIN}',
            Ext_Params::M_DOMAIN         => '{DOMAIN}',
            Ext_Params::M_PORT           => '{PORT}',
            Ext_Params::M_LOGIN          => '{LOGIN}',
            Ext_Params::M_PASSWORD       => '{PASSWORD}',
            Ext_Params::M_TOKEN          => '{TOKEN}',
            Ext_Params::M_INT_ID         => '{INT_ID}',
            Ext_Params::M_HOST           => '{HOST}',
            Ext_Params::M_QUALITY_ID     => '{QUALITY_ID}',
            Ext_Params::M_DEVICE_ID      => '{DEVICE_ID}',
            Ext_Params::M_SERVER_ID      => '{SERVER_ID}',
            Ext_Params::M_VAR1           => '{VAR1}',
            Ext_Params::M_VAR2           => '{VAR2}',
        );

        $channel_custom_url = $channel->get_custom_url();
        $channel_custom_arc_url = $channel->get_custom_archive_template();
        if (empty($channel_custom_url)) {
            // url template, live or archive
            $live_url = $this->get_stream_param($stream_type, Stream_Params::URL_TEMPLATE);

            if (empty($channel_custom_arc_url)) {
                // global url archive template
                $archive_url = $this->get_stream_param($stream_type, Stream_Params::URL_ARC_TEMPLATE);
            } else {
                // custom archive url template
                $archive_url = $channel_custom_arc_url;
            }
        } else {
            // custom url
            $live_url = $channel_custom_url;

            if (empty($channel_custom_arc_url)) {
                // global custom url archive template
                $archive_url = $this->get_stream_param($stream_type, Stream_Params::URL_CUSTOM_ARC_TEMPLATE);
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

        //hd_print("play template: $play_template_url");
        //foreach($ext_params as $key => $value) { hd_print("ext_params: key: $key, value: $value"); }

        // replace all macros
        foreach ($replaces as $key => $value)
        {
            if (isset($ext_params[$key])) {
                $play_template_url = str_replace($value, $ext_params[$key], $play_template_url);
            }
        }

        $url = $this->UpdateDuneParams($play_template_url, $plugin_cookies, $custom_stream_type);

        return HD::make_ts($url);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] | string[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account: $force");

        if (isset($this->account_data) && !$force)
            return $this->account_data;

        $this->account_data = null;
        $this->ClearPlaylistCache();
        $this->SetupM3uParser(true, $plugin_cookies, $force);

        $templates = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $idx = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX);
        $parse_pattern = isset($templates[$idx][Plugin_Constants::PARSE_REGEX]) ? $templates[$idx][Plugin_Constants::PARSE_REGEX] : '';

        if (empty($parse_pattern))
            return false;

        $parse_pattern = "/$parse_pattern/";
        foreach ($this->get_tv_m3u_entries() as $entry) {
            if (preg_match($parse_pattern, $entry->getPath(), $matches)) {
                $this->account_data = $matches;
                return $this->account_data;
            }
        }

        return false;
    }

    /**
     * Collect information from m3u8 playlist
     * @return array
     */
    public function GetPlaylistStreamsInfo()
    {
        $pl_entries = array();
        $templates = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $idx = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX);
        hd_print("Get playlist information for index $idx");

        $parse_pattern = isset($templates[$idx][Plugin_Constants::PARSE_REGEX]) ? $templates[$idx][Plugin_Constants::PARSE_REGEX] : '';
        if (empty($parse_pattern)) {
            hd_print('Empty parse pattern. Unable to parse playlist');
            $this->ClearPlaylistCache();
            return $pl_entries;
        }

        hd_print("Using parsing pattern: $parse_pattern");
        $parse_pattern = "/$parse_pattern/";

        $tag_id = isset($templates[$idx][Plugin_Constants::TAG_ID_MATCH]) ? $templates[$idx][Plugin_Constants::TAG_ID_MATCH] : '';
        hd_print("Using matching tag: $tag_id");

        $m3u_entries = $this->get_tv_m3u_entries();
        hd_print("Parsing " . count($m3u_entries) . "playlist entries");

        foreach ($m3u_entries as $entry) {
            if (!empty($tag_id)) {
                // special case for name, otherwise take ID from selected tag
                $id = ($tag_id === 'name') ? $entry->getTitle() : $entry->getAttribute($tag_id);
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
        } else {
            hd_print("Channels mapped : " . count($pl_entries));
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
        $this->tv_m3u_entries = null;
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
        $this->vod_m3u_indexes = null;
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
     * @param $plugin_cookies
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetchVodCategories($plugin_cookies, &$category_list, &$category_index)
    {
        hd_print("fetch_vod_categories");
        $category_list = array();
        $category_index = array();

        $this->SetupM3uParser(false, $plugin_cookies);

        $t = microtime(1);

        $this->vod_m3u_indexes = $this->m3u_parser->indexFile();
        $all_indexes = array();
        foreach ($this->vod_m3u_indexes as $index_array) {
            foreach ($index_array as $element) {
                $all_indexes[] = $element;
            }
        }
        sort($all_indexes);
        $this->vod_m3u_indexes[Vod_Category::FLAG_ALL] = $all_indexes;

        // all movies
        $count = count($all_indexes);
        $category = new Vod_Category(Vod_Category::FLAG_ALL, "Все фильмы ($count)");
        $category_list[] = $category;
        $category_index[Vod_Category::FLAG_ALL] = $category;

        foreach ($this->vod_m3u_indexes as $group => $indexes) {
            if ($group === Vod_Category::FLAG_ALL) continue;

            $count = count($indexes);
            $cat = new Vod_Category($group, "$group ($count)");
            $category_list[] = $cat;
            $category_index[$group] = $cat;
        }
        hd_print("Categories read: " . count($category_list));
        hd_print("Fetched categories at " . (microtime(1) - $t) . " secs");
        HD::ShowMemoryUsage();
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print("getSearchList: $keyword");

        $vod_pattern = $this->get_vod_parse_pattern($plugin_cookies);
        $t = microtime(1);
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        foreach ($this->vod_m3u_indexes[Vod_Category::FLAG_ALL] as $index) {
            $title = $this->m3u_parser->getTitleByIdx($index);
            if (empty($title)) continue;

            $search_in = utf8_encode(mb_strtolower($title, 'UTF-8'));
            if (strpos($search_in, $keyword) === false) continue;

            if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match)) {
                $title = isset($match['title']) ? $match['title'] : $title;
            }

            $entry = $this->m3u_parser->getEntryByIdx($index);
            if ($entry === null) continue;

            $poster_url = $entry->getAttribute('tvg-logo');
            hd_print("Found at $index movie '$title', poster url: '$poster_url'");
            $movies[] = new Short_Movie($index, $title, $poster_url);
        }

        hd_print("Movies found: " . count($movies));
        hd_print("Search at " . (microtime(1) - $t) . " secs");

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
    public function getMovieList($query_id, $plugin_cookies)
    {
        hd_print("getMovieList: $query_id");
        $movies = array();

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        $current_offset = $this->get_next_page($query_id, 0);
        $indexes = $this->vod_m3u_indexes[$category_id];

        $vod_pattern = $this->get_vod_parse_pattern($plugin_cookies);
        $max = count($indexes);
        $ubound = min($max, $current_offset + 5000);
        hd_print("Read from: $current_offset to $ubound");

        $pos = $current_offset;
        while($pos < $ubound) {
            $index = $indexes[$pos++];
            $entry = $this->m3u_parser->getEntryByIdx($index);
            if ($entry === null) continue;

            $title = $entry->getTitle();
            if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match)) {
                $title = isset($match['title']) ? $match['title'] : $title;
            }

            $movies[] = new Short_Movie($index, $title, $entry->getAttribute('tvg-logo'));
        }

        $this->get_next_page($query_id, $pos - $current_offset);

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
        $movie = new Movie($movie_id, $this->parent);

        $vod_pattern = $this->get_vod_parse_pattern($plugin_cookies);
        $entry = $this->m3u_parser->getEntryByIdx($movie_id);
        if ($entry === null) {
            hd_print("Movie not found");
        } else {
            $logo = $entry->getAttribute('tvg-logo');
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

            $category = '';
            foreach ($this->vod_m3u_indexes as $group => $indexes) {
                if ($group === Vod_Category::FLAG_ALL) continue;
                if (in_array($movie_id, $indexes)) {
                    $category = $group;
                    break;
                }
            }

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
                $category,// $xml->genres,
                '',// $xml->rate_imdb,
                '',// $xml->rate_kinopoisk,
                '',// $xml->rate_mpaa,
                $country,// $xml->country,
                ''// $xml->budget
            );

            $movie->add_series_data($movie_id, $title, '', $entry->getPath());
            //hd_print("Vod url: " . $entry->getPath());
        }

        return $movie;
    }

    /**
     * @param string $url
     * @param $plugin_cookies
     * @param int $custom_type
     * @return string
     */
    public function UpdateDuneParams($url, $plugin_cookies, $custom_type = '')
    {
        $type = empty($custom_type) ? $this->get_format($plugin_cookies) : $custom_type;

        $dune_params = $this->get_stream_param($type, Stream_Params::DUNE_PARAMS);
        if (!empty($dune_params)) {
            hd_print("Additional dune params: $dune_params");
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
            $dune_params = trim($dune_params, '|');
            $dune_params = str_replace('{BUFFERING}', $buf_time, $dune_params);
            $url .= "|||dune_params|||$dune_params";
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
     * @return string
     */
    protected function FetchTvM3U($plugin_cookies, $force = false)
    {
        $m3u_file = get_temp_path($this->PluginShortName . "_playlist_tv.m3u8");
        if ($force === false) {
            if (file_exists($m3u_file)) {
                $mtime = filemtime($m3u_file);
                if (time() - $mtime > 3600) {
                    hd_print("Playlist cache expired. Forcing reload");
                    $force = true;
                }
            } else {
                $force = true;
            }
        }

        if ($force !== false) {
            try {
                $url = $this->GetPlaylistUrl($plugin_cookies);
                //hd_print("tv1 m3u8 playlist: " . $url);
                if (empty($url)) {
                    hd_print("Tv playlist not defined");
                    throw new Exception('Tv playlist not defined');
                }
                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print("Unable to load tv playlist: " . $ex->getMessage());
            }
        }

        return $m3u_file;
    }

    /**
     * @param $plugin_cookies
     * @param bool $force
     * @return string
     */
    protected function FetchVodM3U($plugin_cookies, $force = false)
    {
        $m3u_file = get_temp_path($this->PluginShortName . "_playlist_vod.m3u8");
        if ($force === false) {
            if (file_exists($m3u_file)) {
                $mtime = filemtime($m3u_file);
                if (time() - $mtime > 3600) {
                    hd_print("VOD playlist cache expired. Forcing reload");
                    $force = true;
                }
            } else {
                $force = true;
            }
        }

        if ($force !== false) {
            try {
                $url = $this->GetVodListUrl($plugin_cookies);
                if (empty($url)) {
                    hd_print("Vod playlist not defined");
                    throw new Exception("Vod playlist not defined");
                }

                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print("Unable to load movie playlist: " . $ex->getMessage());
            }
        }

        return $m3u_file;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($plugin_cookies)
    {
        $templates = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $idx = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX);
        $url = isset($templates[$idx][Plugin_Constants::URI_TEMPLATE]) ? $templates[$idx][Plugin_Constants::URI_TEMPLATE] : '';
        return $this->replace_subs_vars($url, $plugin_cookies);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        return $this->replace_subs_vars($this->get_vod_template($plugin_cookies), $plugin_cookies);
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

            if (isset($plugin_cookies->subdomain) && strpos($url, '{SUBDOMAIN}') !== false) {
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

    public function get_vod_template($plugin_cookies)
    {
        $idx = isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
        $vod_templates = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        return isset($vod_templates[$idx][Plugin_Constants::URI_TEMPLATE]) ? $vod_templates[$idx][Plugin_Constants::URI_TEMPLATE] : '';
    }

    public function get_vod_template_name($plugin_cookies)
    {
        $idx = isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
        $vod_templates = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        return isset($vod_templates[$idx][Plugin_Constants::PLAYLIST_NAME]) ? $vod_templates[$idx][Plugin_Constants::PLAYLIST_NAME] : '';
    }

    protected function get_vod_parse_pattern($plugin_cookies)
    {
        $idx = isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
        $vod_templates = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        $vod_pattern = isset($vod_templates[$idx][Plugin_Constants::PARSE_REGEX]) ? $vod_templates[$idx][Plugin_Constants::PARSE_REGEX] : '';
        if (!empty($vod_pattern))
            $vod_pattern = "/$vod_pattern/";

        return $vod_pattern;
    }
}

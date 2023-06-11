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
    protected $last_error;

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

    public function init_plugin()
    {
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
     * @return mixed
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
     * @param $server int
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
     * @return int
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
            $plugin_cookies->channels_list = sprintf('%s_channel_list.xml', $this->plugin_info['app_type_name']);
        }
        $used_list = $plugin_cookies->channels_list;

        if (!isset($plugin_cookies->channels_source)) {
            $plugin_cookies->channels_source = 1;
        }

        switch ($plugin_cookies->channels_source) {
            case 1: // folder
                hd_print("Channels source: folder");
                $channels_list_path = smb_tree::get_folder_info($plugin_cookies, Starnet_Folder_Screen::ACTION_CH_LIST_PATH);
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

    /**
     * @param $plugin_cookies
     * @return int
     */
    public function get_tv_list_idx($plugin_cookies)
    {
        return isset($plugin_cookies->playlist_idx) ? $plugin_cookies->playlist_idx : $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX);
    }

    /**
     * @param $plugin_cookies
     * @param $current_idx
     * @return array
     */
    public function get_tv_list_names($plugin_cookies, &$current_idx)
    {
        $tv_lists_array = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $current_idx = $this->get_tv_list_idx($plugin_cookies);
        $tv_lists = array();
        foreach ($tv_lists_array as $list) {
            $tv_lists[] = $list[Plugin_Constants::PLAYLIST_NAME];
        }
        return $tv_lists;
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_current_tv_template($plugin_cookies)
    {
        $tv_lists_array = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $current_idx = $this->get_tv_list_idx($plugin_cookies);
        return isset($tv_lists_array[$current_idx]) ? $tv_lists_array[$current_idx] : array();
    }

    /**
     * @param $plugin_cookies
     * @return int
     */
    public function get_vod_list_idx($plugin_cookies)
    {
        return isset($plugin_cookies->vod_idx) ? $plugin_cookies->vod_idx : 0;
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_current_vod_template($plugin_cookies)
    {
        $current_idx = $this->get_vod_list_idx($plugin_cookies);
        $vod_templates = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        return isset($vod_templates[$current_idx]) ? $vod_templates[$current_idx] : array();
    }

    public function get_vod_list_names($plugin_cookies, &$current_idx)
    {
        $vod_lists_array = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        $current_idx = $this->get_vod_list_idx($plugin_cookies);
        $vod_lists = array();
        foreach ($vod_lists_array as $list) {
            $vod_lists[] = $list[Plugin_Constants::PLAYLIST_NAME];
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
     * @return string
     */
    public function get_last_error()
    {
        return $this->last_error;
    }

    /**
     * @param string $error
     */
    public function set_last_error($error)
    {
        $this->last_error = $error;
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
        Control_Factory::add_label($defs, TR::t('balance'), TR::t('balance_not_support'));
    }

    /**
     * Generate url from template with macros substitution
     * Make url ts wrapped
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     * @throws Exception
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $now = time();
        $is_archive = (int)$archive_ts > 0;
        $stream_type = $this->get_format($plugin_cookies);
        $ext_params = $channel->get_ext_params();
        $channel_id = $channel->get_channel_id();
        $ext_params[Plugin_Constants::CGI_BIN] = get_plugin_cgi_url();
        $ext_params[Plugin_Constants::CHANNEL_ID] = $channel_id;
        $ext_params[Stream_Params::CU_START] = $archive_ts;
        $ext_params[Stream_Params::CU_NOW] = $now;
        $ext_params[Stream_Params::CU_OFFSET] = $now - $archive_ts;
        $ext_params[Stream_Params::CU_STOP] = $archive_ts + $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);
        $ext_params[Stream_Params::CU_DURATION] = $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);
        $ext_params[Ext_Params::M_DEVICE_ID] = $this->get_device_id($plugin_cookies);
        $ext_params[Ext_Params::M_SERVER_ID] = $this->get_server_id($plugin_cookies);

        $replaces = array(
            Plugin_Constants::CGI_BIN    => Plugin_Macros::CGI_BIN,
            Plugin_Constants::CHANNEL_ID => Plugin_Macros::ID,
            Stream_Params::CU_START      => Plugin_Macros::START,
            Stream_Params::CU_NOW        => Plugin_Macros::NOW,
            Stream_Params::CU_DURATION   => Plugin_Macros::DURATION,
            Stream_Params::CU_STOP       => Plugin_Macros::STOP,
            Stream_Params::CU_OFFSET     => Plugin_Macros::OFFSET,
            Ext_Params::M_SUBDOMAIN      => Plugin_Macros::SUBDOMAIN,
            Ext_Params::M_DOMAIN         => Plugin_Macros::DOMAIN,
            Ext_Params::M_PORT           => Plugin_Macros::PORT,
            Ext_Params::M_LOGIN          => Plugin_Macros::LOGIN,
            Ext_Params::M_PASSWORD       => Plugin_Macros::PASSWORD,
            Ext_Params::M_TOKEN          => Plugin_Macros::TOKEN,
            Ext_Params::M_INT_ID         => Plugin_Macros::INT_ID,
            Ext_Params::M_HOST           => Plugin_Macros::HOST,
            Ext_Params::M_QUALITY_ID     => Plugin_Macros::QUALITY_ID,
            Ext_Params::M_DEVICE_ID      => Plugin_Macros::DEVICE_ID,
            Ext_Params::M_SERVER_ID      => Plugin_Macros::SERVER_ID,
            Ext_Params::M_VAR1           => Plugin_Macros::VAR1,
            Ext_Params::M_VAR2           => Plugin_Macros::VAR2,
            Ext_Params::M_VAR3           => Plugin_Macros::VAR3,
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
            $play_template_url = str_replace(Plugin_Macros::LIVE_URL, $live_url, $archive_url);
            $custom_stream_type = $channel->get_custom_archive_url_type();
        } else {
            $play_template_url = $live_url;
            $custom_stream_type = $channel->get_custom_url_type();
        }

        //hd_print("play template: $play_template_url");
        //foreach($ext_params as $key => $value) { hd_print("ext_params: key: $key, value: $value"); }

        // replace all macros
        foreach ($replaces as $key => $value) {
            if (isset($ext_params[$key])) {
                $play_template_url = str_replace($value, $ext_params[$key], $play_template_url);
            }
        }

        foreach ($replaces as $value) {
            if (strpos($play_template_url, $value) !== false) {
                throw new Exception("Template $value not replaced. Url not generated.");
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
        hd_print(__METHOD__ . ": Collect information from account: $force");

        if (isset($this->account_data) && !$force)
            return $this->account_data;

        $this->account_data = null;
        $this->ClearPlaylistCache($plugin_cookies);
        $this->SetupM3uParser(true, $plugin_cookies, $force);

        $parse_pattern = $this->get_tv_parse_pattern($plugin_cookies);
        if (empty($parse_pattern))
            return false;

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
    public function GetPlaylistStreamsInfo($plugin_cookies)
    {
        $pl_entries = array();

        $parse_pattern = $this->get_tv_parse_pattern($plugin_cookies);
        if (empty($parse_pattern)) {
            hd_print(__METHOD__ . ": Empty tv parsing pattern!");
        }

        $template = $this->get_current_tv_template($plugin_cookies);
        $tag_id = isset($template[Plugin_Constants::TAG_ID_MATCH]) ? $template[Plugin_Constants::TAG_ID_MATCH] : '';
        if (!empty($tag_id)) {
            hd_print(__METHOD__ . ": ID matching tag: $tag_id");
        }

        $m3u_entries = $this->get_tv_m3u_entries();
        $total = count($m3u_entries);
        hd_print(__METHOD__ . ": Parsing $total playlist entries");

        $mapped = 0;
        foreach ($m3u_entries as $entry) {
            if (!empty($tag_id)) {
                // special case for name, otherwise take ID from selected tag
                $id = ($tag_id === 'name') ? $entry->getTitle() : $entry->getAttribute($tag_id);
                if (empty($id)) {
                    hd_print(__METHOD__ . ": Unable to map ID by $tag_id for entry with url: " . $entry->getPath());
                    continue;
                }
            }

            // http://some_domain/some_token/index.m3u8
            if (!empty($parse_pattern) && preg_match($parse_pattern, $entry->getPath(), $matches)) {
                $id = !empty($tag_id) ? $id : $matches['id'];
                $pl_entries[$id] = $matches;
                $mapped++;
            } else {
                // threat as custom url
                $pl_entries[hash('crc32', $entry->getPath())] = array();
            }
        }

        if (empty($pl_entries) && $this->plugin_info['app_type_name'] !== 'custom') {
            $this->set_last_error("Пустой плейлист провайдера!");
            hd_print($this->last_error);
            $this->ClearPlaylistCache($plugin_cookies);
        } else {
            hd_print(__METHOD__ . ": Total entries:" . count($pl_entries) . ", mapped to ID $mapped: ");
        }

        return $pl_entries;
    }

    /**
     * Clear downloaded playlist
     * @param $plugin_cookies
     * @return void
     */
    public function ClearPlaylistCache($plugin_cookies)
    {
        $tmp_file = get_temp_path($this->get_tv_list_idx($plugin_cookies) . "_playlist_tv.m3u8");
        $this->tv_m3u_entries = null;
        hd_print(__METHOD__ . ": $tmp_file");
        if (file_exists($tmp_file)) {
            copy($tmp_file, $tmp_file . ".m3u");
            unlink($tmp_file);
        }
    }

    /**
     * Clear downloaded playlist
     * @param $plugin_cookies
     * @return void
     */
    public function ClearVodCache($plugin_cookies)
    {
        $tmp_file = get_temp_path($this->get_vod_list_idx($plugin_cookies) . "_playlist_vod.m3u8");
        $bak_file = $tmp_file . ".bak";
        if (file_exists($tmp_file)) {
            copy($tmp_file, $bak_file);
        }
        hd_print(__METHOD__ . ": $tmp_file");
        if (file_exists($tmp_file)) {
            copy($tmp_file, $tmp_file . ".m3u");
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
        hd_print(__METHOD__ . ": $tmp_file");
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
        hd_print(__METHOD__);
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
        hd_print(__METHOD__ . ": Categories read: " . count($category_list));
        hd_print(__METHOD__ . ": Fetched categories at " . (microtime(1) - $t) . " secs");
        HD::ShowMemoryUsage();
    }

    /**
     * @param string $keyword
     * @param $plugin_cookies
     * @return array
     */
    public function getSearchList($keyword, $plugin_cookies)
    {
        hd_print(__METHOD__ . ": $keyword");

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
            hd_print(__METHOD__ . ": Found at $index movie '$title', poster url: '$poster_url'");
            $movies[] = new Short_Movie($index, $title, $poster_url);
        }

        hd_print(__METHOD__ . ": Movies found: " . count($movies));
        hd_print(__METHOD__ . ": Search at " . (microtime(1) - $t) . " secs");

        return $movies;
    }

    /**
     * @param string $params
     * @param $plugin_cookies
     * @return array
     */
    public function getFilterList($params, $plugin_cookies)
    {
        //hd_print(__METHOD__ . ": $params");
        return array();
    }

    /**
     * @param string $query_id
     * @param $plugin_cookies
     * @return array
     */
    public function getMovieList($query_id, $plugin_cookies)
    {
        hd_print(__METHOD__ . ": $query_id");
        $movies = array();

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        $current_offset = $this->get_next_page($query_id, 0);
        $indexes = $this->vod_m3u_indexes[$category_id];

        $vod_pattern = $this->get_vod_parse_pattern($plugin_cookies);
        $max = count($indexes);
        $ubound = min($max, $current_offset + 5000);
        hd_print(__METHOD__ . ": Read from: $current_offset to $ubound");

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
        hd_print(__METHOD__ . ": $movie_id");
        $movie = new Movie($movie_id, $this->parent);

        $vod_pattern = $this->get_vod_parse_pattern($plugin_cookies);
        $entry = $this->m3u_parser->getEntryByIdx($movie_id);
        if ($entry === null) {
            hd_print(__METHOD__ . ": Movie not found");
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
     * @return string
     */
    public function UpdateVodUrlParams($url, $plugin_cookies)
    {
        $vod_template = $this->get_current_vod_template($plugin_cookies);
        $url_prefix = isset($vod_template[Plugin_Constants::URL_PREFIX]) ? $vod_template[Plugin_Constants::URL_PREFIX] : '';

        $url_prefix = str_replace(Plugin_Macros::CGI_BIN, get_plugin_cgi_url(), $url_prefix);
        if (!empty($url_prefix)) {
            $url = $url_prefix . $url;
        }

        $url_params = isset($vod_template[Plugin_Constants::URL_PARAMS]) ? $vod_template[Plugin_Constants::URL_PARAMS] : '';
        if ($url_params) {
            $url .= $url_params;
        }

        return $url;
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
            //hd_print("Additional dune params: $dune_params");
            $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
            $dune_params = trim($dune_params, '|');
            $dune_params = str_replace(Plugin_Macros::BUFFERING, $buf_time, $dune_params);
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
        $m3u_file = get_temp_path($this->get_tv_list_idx($plugin_cookies) . "_playlist_tv.m3u8");
        if ($force === false) {
            if (file_exists($m3u_file)) {
                $mtime = filemtime($m3u_file);
                if (time() - $mtime > 3600) {
                    hd_print(__METHOD__ . ": Playlist cache expired. Forcing reload");
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
                    hd_print(__METHOD__ . "Tv playlist not defined");
                    throw new Exception('Tv playlist not defined');
                }
                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print(__METHOD__ . ": Unable to load tv playlist: " . $ex->getMessage());
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
        $m3u_file = get_temp_path($this->get_vod_list_idx($plugin_cookies) . "_playlist_vod.m3u8");
        if ($force === false) {
            if (file_exists($m3u_file)) {
                $mtime = filemtime($m3u_file);
                if (time() - $mtime > 3600) {
                    hd_print(__METHOD__ . ": VOD playlist cache expired. Forcing reload");
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
                    hd_print(__METHOD__ . ": Vod playlist not defined");
                    throw new Exception("Vod playlist not defined");
                }

                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_print(__METHOD__ . ": Unable to load movie playlist: " . $ex->getMessage());
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
        hd_print(__METHOD__ . ": Get playlist url for " . $this->get_tv_template_name($plugin_cookies));
        $template = $this->get_current_tv_template($plugin_cookies);
        $url = isset($template[Plugin_Constants::PL_TEMPLATE]) ? $template[Plugin_Constants::PL_TEMPLATE] : '';
        return $this->replace_subs_vars($url, $plugin_cookies);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        return $this->replace_subs_vars($this->get_vod_uri($plugin_cookies), $plugin_cookies);
    }

    /**
     * @param string $url
     * @param $plugin_cookies
     * @return string
     */
    protected function replace_subs_vars($url, $plugin_cookies)
    {
        if (!empty($url)) {

            if (strpos($url, Plugin_Macros::API_URL) !== false) {
                $api_url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL);
                if (empty($api_url))
                    hd_print(__METHOD__ . ": Provider API url not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::API_URL, $api_url, $url);
            }

            if (strpos($url, Plugin_Macros::PL_DOMAIN) !== false) {
                $tv_template = $this->get_current_tv_template($plugin_cookies);
                $pl_domain = isset($tv_template[Plugin_Constants::PL_DOMAIN]) ? $tv_template[Plugin_Constants::PL_DOMAIN] : '';
                if (empty($pl_domain))
                    hd_print(__METHOD__ . ": Playlist domain not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::PL_DOMAIN, $pl_domain, $url);
            }

            if (strpos($url, Plugin_Macros::VOD_DOMAIN) !== false) {
                $vod_template = $this->get_current_vod_template($plugin_cookies);
                $vod_domain = isset($vod_template[Plugin_Constants::PL_DOMAIN]) ? $vod_template[Plugin_Constants::PL_DOMAIN] : '';
                if (empty($vod_domain))
                    hd_print(__METHOD__ . ": Vod domain not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::VOD_DOMAIN, $vod_domain, $url);
            }

            if (strpos($url, Plugin_Macros::SUBDOMAIN) !== false) {
                if (!isset($plugin_cookies->subdomain)) {
                    hd_print(__METHOD__ . ": Subdomain not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::SUBDOMAIN, $plugin_cookies->subdomain, $url);
                }
            }

            if (strpos($url, Plugin_Macros::LOGIN) !== false) {
                $login = $this->get_login($plugin_cookies);
                if (empty($login))
                    hd_print(__METHOD__ . ": Login not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::LOGIN, $login, $url);
            }

            if (strpos($url, Plugin_Macros::PASSWORD) !== false) {
                $password = $this->get_password($plugin_cookies);
                if (empty($password))
                    hd_print(__METHOD__ . ": Password not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::PASSWORD, $password, $url);
            }

            if (strpos($url, Plugin_Macros::TOKEN) !== false) {
                $this->ensure_token_loaded($plugin_cookies);
                if (empty($plugin_cookies->token))
                    hd_print(__METHOD__ . ": Token not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::TOKEN, $plugin_cookies->token, $url);
            }

            if (strpos($url, Plugin_Macros::SERVER) !== false) {
                $server = $this->get_server_name($plugin_cookies);
                if (empty($server))
                    hd_print(__METHOD__ . ": Server not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::SERVER, $server, $url);
            }

            if (strpos($url, Plugin_Macros::SERVER_ID) !== false) {
                $server_id = $this->get_server_id($plugin_cookies);
                if (empty($server_id))
                    hd_print(__METHOD__ . ": Server ID not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::SERVER_ID, $server_id, $url);
            }

            if (strpos($url, Plugin_Macros::QUALITY_ID) !== false) {
                $quality = $this->get_quality_id($plugin_cookies);
                if (empty($quality))
                    hd_print(__METHOD__ . ": Quality ID not set, but macro was used");
                else
                    $url = str_replace(Plugin_Macros::QUALITY_ID, $quality, $url);
            }

            if (strpos($url, Plugin_Macros::DEVICE_ID) !== false) {
                $device = $this->get_device_id($plugin_cookies);
                if (empty($device))
                    hd_print(__METHOD__ . ": Device ID not set, but macro was used");
                else
                $url = str_replace(Plugin_Macros::DEVICE_ID, $device, $url);
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

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_template_name($plugin_cookies)
    {
        $template = $this->get_current_tv_template($plugin_cookies);
        return isset($template[Plugin_Constants::PLAYLIST_NAME]) ? $template[Plugin_Constants::PLAYLIST_NAME] : $this->plugin_info['app_type_name'];
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_tv_parse_pattern($plugin_cookies)
    {
        $template = $this->get_current_tv_template($plugin_cookies);
        $parse_pattern = isset($template[Plugin_Constants::PARSE_REGEX]) ? $template[Plugin_Constants::PARSE_REGEX] : '';
        if (!empty($parse_pattern))
            $parse_pattern = "/$parse_pattern/";

        return $parse_pattern;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_uri($plugin_cookies)
    {
        $vod_template = $this->get_current_vod_template($plugin_cookies);
        return isset($vod_template[Plugin_Constants::PL_TEMPLATE]) ? $vod_template[Plugin_Constants::PL_TEMPLATE] : '';
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_template_name($plugin_cookies)
    {
        $vod_template = $this->get_current_vod_template($plugin_cookies);
        return isset($vod_template[Plugin_Constants::PLAYLIST_NAME]) ? $vod_template[Plugin_Constants::PLAYLIST_NAME] : $this->plugin_info['app_type_name'];
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_vod_parse_pattern($plugin_cookies)
    {
        $vod_template = $this->get_current_vod_template($plugin_cookies);
        $vod_pattern = isset($vod_template[Plugin_Constants::PARSE_REGEX]) ? $vod_template[Plugin_Constants::PARSE_REGEX] : '';
        if (!empty($vod_pattern))
            $vod_pattern = "/$vod_pattern/";

        return $vod_pattern;
    }
}

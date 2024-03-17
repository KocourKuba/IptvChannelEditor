<?php

require_once 'default_dune_plugin.php';
require_once 'dynamic_config.php';
require_once 'epg_manager_json.php';
require_once 'epg_manager_sql.php';
require_once 'channel.php';
require_once 'm3u/M3uParser.php';

class default_config extends dynamic_config
{
    // page counter for some plugins
    protected $pages = array();
    protected $is_entered = false;
    protected $movie_counter = array();
    protected $filters = array();

    /**
     * @var Object|null
     */
    protected $embedded_account;

    /**
     * @var string
     */
    protected $last_error;

    /**
     * @var array
     */
    protected $account_data;

    /**
     * @var Default_Dune_Plugin
     */
    protected $plugin;

    /**
     * @var Entry[]
     */
    protected $tv_m3u_entries;

    /**
     * @var array[]
     */
    protected $vod_m3u_indexes;

    public function set_plugin($plugin)
    {
        $this->plugin = $plugin;
    }

    public function load_embedded_account()
    {
        hd_debug_print("Loading embedded account");
        $plugin_account = get_install_path('account.dat');
        $plugin_data = '';
        if (file_exists($plugin_account)) {
            $plugin_data = file_get_contents($plugin_account, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            hd_debug_print("account data: $plugin_data");
            if ($plugin_data !== false) {
                $plugin_data = base64_decode(substr($plugin_data, 5));
            }
        }

        $backup_account = get_data_path('account.dat');
        $backup_data = '';
        if (file_exists($backup_account)) {
            $backup_data = file_get_contents($backup_account, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        }

        if (empty($plugin_data) && empty($backup_data)) {
            hd_debug_print("No embedded account in main and backup locations");
            return;
        }

        if (!empty($plugin_data)) {
            if ($plugin_data !== $backup_data) {
                hd_debug_print("backup main account data.");
                $backup_data = $plugin_data;
                file_put_contents($backup_account, $backup_data);
            }
        } else if (!empty($backup_data)) {
            hd_debug_print("using backup account data.");
            $plugin_data = $backup_data;
        }

        $account = json_decode($plugin_data);
        if ($account !== false) {
            hd_debug_print("account data loaded.");
            $this->embedded_account = $account;
        }
    }

    public function init_custom_config()
    {
    }

    /**
     * @return Entry[]
     */
    public function get_tv_m3u_entries()
    {
        hd_debug_print(null, true);
        if (empty($this->tv_m3u_entries)) {
            if ($this->plugin->get_m3u_parser()->parseInMemory()) {
                $this->tv_m3u_entries = $this->plugin->get_m3u_parser()->getM3uEntries();
                hd_debug_print("Total entries loaded from playlist m3u file:" . count($this->tv_m3u_entries));
                HD::ShowMemoryUsage();
            } else {
                hd_debug_print("Failed to parse M3U file");
                return array();
            }
        }

        return $this->tv_m3u_entries;
    }

    /**
     * @return Object|null
     */
    public function get_embedded_account()
    {
        return isset($this->embedded_account) ? $this->embedded_account : null;
    }

    /**
     * @param Object|null $val
     */
    public function set_embedded_account($val)
    {
        $this->embedded_account = $val;
    }

    /**
     * @return string
     */
    public function get_login()
    {
        return isset($this->embedded_account->login) ? $this->embedded_account->login : $this->plugin->get_credentials(Ext_Params::M_LOGIN);
    }

    /**
     * @return string
     */
    public function get_password()
    {
        return isset($this->embedded_account->password) ? $this->embedded_account->password : $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
    }

    /**
     * @return string
     */
    public function get_ott_key()
    {
        return null;
    }

    /**
     * @return string
     */
    public function get_server_name()
    {
        $servers = $this->get_servers();
        return $servers[$this->get_server_id()];
    }

    /**
     * @return mixed
     */
    public function get_server_id()
    {
        $server = $this->plugin->get_parameter(Ext_Params::M_SERVER_ID);
        $embedded_acc = $this->get_embedded_account();
        if (!is_null($embedded_acc) && isset($embedded_acc->server_id) && is_null($server)) {
            $this->plugin->set_parameter(Ext_Params::M_SERVER_ID, $embedded_acc->server_id);
            $server = $embedded_acc->server_id;
        }

        $servers = $this->get_servers();
        reset($servers);
        return !is_null($server) && isset($servers[$server]) ? $server : key($servers);
    }

    /**
     * @param $server int
     */
    public function set_server_id($server)
    {
        $this->plugin->set_parameter(Ext_Params::M_SERVER_ID, $server);
    }

    /**
     * @return string
     */
    public function get_device_name()
    {
        $devices = $this->get_devices();
        return $devices[$this->get_device_id()];
    }

    /**
     * @return int
     */
    public function get_device_id()
    {
        $embedded_acc = $this->get_embedded_account();
        $device = $this->plugin->get_parameter(Ext_Params::M_DEVICE_ID);
        if (!is_null($embedded_acc) && isset($embedded_acc->device_id) && empty($device)) {
            $this->plugin->set_parameter(Ext_Params::M_DEVICE_ID, $embedded_acc->device_id);
            $device = $embedded_acc->device_id;
        }

        $devices = $this->get_devices();
        return !empty($device) && isset($devices[$device]) ? $device : key($devices);
    }

    /**
     * @param $device_id
     */
    public function set_device_id($device_id)
    {
        $this->plugin->set_parameter(Ext_Params::M_DEVICE_ID, $device_id);
    }

    /**
     * @return string
     */
    public function get_quality_name()
    {
        $qualities = $this->get_qualities();
        return $qualities[$this->get_quality_id()];
    }

    /**
     * @return mixed|null
     */
    public function get_quality_id()
    {
        $embedded_acc = $this->get_embedded_account();
        $quality = $this->plugin->get_parameter(Ext_Params::M_QUALITY_ID);
        if (!is_null($embedded_acc) && isset($embedded_acc->quality_id) && empty($quality)) {
            $this->plugin->set_parameter(Ext_Params::M_QUALITY_ID, $embedded_acc->quality_id);
            $quality = $embedded_acc->quality_id;
        }

        $qualities = $this->get_qualities();
        reset($qualities);
        return !empty($quality) && isset($qualities[$quality]) ? $quality : key($qualities);
    }

    /**
     * @param $quality_id
     */
    public function set_quality_id($quality_id)
    {
        $this->plugin->set_parameter(Ext_Params::M_QUALITY_ID, $quality_id);
    }

    /**
     * @return string
     */
    public function get_profile_name()
    {
        $profiles = $this->get_profiles();
        return $profiles[$this->get_profile_id()];
    }

    /**
     * @return string|null
     */
    public function get_profile_id()
    {
        $embedded_acc = $this->get_embedded_account();
        $profile = $this->plugin->get_parameter(Ext_Params::M_PROFILE_ID);
        if (!is_null($embedded_acc) && isset($embedded_acc->device_id) && empty($profile)) {
            $this->plugin->set_parameter(Ext_Params::M_PROFILE_ID, $embedded_acc->profile_id);
            $profile = $embedded_acc->profile_id;
        }

        $profiles = $this->get_profiles();
        reset($profiles);
        return !empty($profile) && isset($profiles[$profile]) ? $profile : key($profiles);
    }

    /**
     * @param $profile_id
     */
    public function set_profile_id($profile_id)
    {
        $this->plugin->set_parameter(Ext_Params::M_PROFILE_ID, $profile_id);
    }

    /**
     * @return string
     */
    public function get_domain_name()
    {
        $domains = $this->get_domains();
        return $domains[$this->get_domain_id()];
    }

    /**
     * @return string|null
     */
    public function get_domain_id()
    {
        $embedded_acc = $this->get_embedded_account();
        $domain_id = $this->plugin->get_parameter(Ext_Params::M_DOMAIN_ID);
        if (!is_null($embedded_acc) && isset($embedded_acc->domain_id) && empty($domain_id)) {
            $this->plugin->set_parameter(Ext_Params::M_DOMAIN_ID, $embedded_acc->domain_id);
            $domain_id = $embedded_acc->domain_id;
        }

        $domains = $this->get_domains();
        return !empty($domain_id) && isset($domains[$domain_id]) ? $domain_id : key($domains);
    }

    /**
     * @param $domain_id
     */
    public function set_domain_id($domain_id)
    {
        $this->plugin->set_parameter(Ext_Params::M_DOMAIN_ID, $domain_id);
    }

    /**
     * @param string &$used_list
     * @return array $all_channels
     */
    public function get_channel_list(&$used_list)
    {
        $channels_list_name = $this->plugin->get_parameter(PARAM_CHANNELS_LIST_NAME);
        if (empty($channels_list_name)) {
            $used_list = sprintf('%s_channel_list.xml', $this->plugin_info['app_type_name']);
        } else {
            $used_list = $channels_list_name;
        }

        $channels_source = $this->plugin->get_parameter(PARAM_CHANNELS_SOURCE, 1);

        switch ($channels_source) {
            case 1: // folder
                hd_debug_print("Channels source: folder");
                $channels_list_path = smb_tree::get_folder_info($this->plugin->get_parameter(PARAM_CHANNELS_LIST_PATH, get_install_path()));
                break;
            case 2: // url
                hd_debug_print("Channels source: url");
                $channels_list_path = get_install_path();
                break;
            case 3: // direct url
                hd_debug_print("Channels source: direct url");
                $channels_list_path = get_install_path();
                break;
            default:
                return array();
        }

        hd_debug_print("Channels list search path: $channels_list_path");

        $all_channels = array();
        $list = glob_dir($channels_list_path, '/\.xml$/i');
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename !== 'dune_plugin.xml') {
                hd_debug_print("Found channels list: $filename");
                $all_channels[$filename] = $filename;
            }
        }

        if (empty($all_channels)) {
            hd_debug_print("No channels list found in selected location: " . $channels_list_path);
            return $all_channels;
        }

        if (!in_array($used_list, $all_channels)) {
            $used_list = (string)reset($all_channels);
        }

        hd_debug_print("Used channels list: $used_list");
        return $all_channels;
    }

    /**
     * @return int
     */
    public function get_tv_list_idx()
    {
        $playlist_idx = $this->plugin->get_parameter(PARAM_PLAYLIST_IDX);
        return empty($playlist_idx) ? $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX) : $playlist_idx;
    }

    /**
     * @param $current_idx
     * @return array
     */
    public function get_tv_list_names(&$current_idx)
    {
        $tv_lists_array = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $current_idx = $this->get_tv_list_idx();
        $tv_lists = array();
        foreach ($tv_lists_array as $list) {
            $tv_lists[] = $list[Plugin_Constants::PLAYLIST_NAME];
        }
        return $tv_lists;
    }

    /**
     * @return array
     */
    public function get_current_tv_template()
    {
        $tv_lists_array = $this->get_feature(Plugin_Constants::PLAYLIST_TEMPLATES);
        $current_idx = $this->get_tv_list_idx();
        return isset($tv_lists_array[$current_idx]) ? $tv_lists_array[$current_idx] : array();
    }

    /**
     * @return int
     */
    public function get_vod_list_idx()
    {
        $vod_idx = $this->plugin->get_parameter(PARAM_VOD_IDX);
        return empty($vod_idx) ? 0 : $vod_idx;
    }

    /**
     * @return array
     */
    public function get_current_vod_template()
    {
        $current_idx = $this->get_vod_list_idx();
        $vod_templates = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        return isset($vod_templates[$current_idx]) ? $vod_templates[$current_idx] : array();
    }

    /**
     * @param int &$current_idx
     * @return array
     */
    public function get_vod_list_names(&$current_idx)
    {
        $vod_lists_array = $this->get_feature(Plugin_Constants::VOD_TEMPLATES);
        $current_idx = $this->get_vod_list_idx();
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
     */
    public function AddSubscriptionUI(&$defs)
    {
        Control_Factory::add_label($defs, TR::t('balance'), TR::t('balance_not_support'));
    }

    /**
     * Generate url from template with macros substitution
     * Make url ts wrapped
     * @param Channel $channel
     * @param int $archive_ts
     * @param bool $clean
     * @return string
     * @throws Exception
     */
    public function GenerateStreamUrl(Channel $channel, $archive_ts, $clean = false)
    {
        $now = time();
        $is_archive = (int)$archive_ts > 0;
        $stream_type = $this->get_format();
        $ext_params = $channel->get_ext_params();
        $channel_id = $channel->get_channel_id();
        $ext_params[Plugin_Constants::CGI_BIN] = get_plugin_cgi_url();
        $ext_params[Plugin_Constants::CHANNEL_ID] = $channel_id;
        $ext_params[Stream_Params::CU_START] = $archive_ts;
        $ext_params[Stream_Params::CU_NOW] = $now;
        $ext_params[Stream_Params::CU_OFFSET] = $now - $archive_ts;
        $ext_params[Stream_Params::CU_STOP] = $archive_ts + $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);
        $ext_params[Stream_Params::CU_DURATION] = $this->get_stream_param($stream_type, Stream_Params::CU_DURATION);

        $replaces = array(
            Plugin_Constants::CGI_BIN    => Plugin_Macros::CGI_BIN,
            Plugin_Constants::CHANNEL_ID => Plugin_Macros::ID,
            Stream_Params::CU_START      => Plugin_Macros::START,
            Stream_Params::CU_NOW        => Plugin_Macros::NOW,
            Stream_Params::CU_DURATION   => Plugin_Macros::DURATION,
            Stream_Params::CU_STOP       => Plugin_Macros::STOP,
            Stream_Params::CU_OFFSET     => Plugin_Macros::OFFSET,
            Ext_Params::M_SCHEME         => Plugin_Macros::SCHEME,
            Ext_Params::M_DOMAIN         => Plugin_Macros::DOMAIN,
            Ext_Params::M_PORT           => Plugin_Macros::PORT,
            Ext_Params::M_TOKEN          => Plugin_Macros::TOKEN,
            Ext_Params::M_INT_ID         => Plugin_Macros::INT_ID,
            Ext_Params::M_HOST           => Plugin_Macros::HOST,
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

        $play_template_url = $this->replace_account_vars($play_template_url);

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

        if ($clean) {
            return $play_template_url;
        }

        $url = $this->UpdateDuneParams($play_template_url, $custom_stream_type);
        return HD::make_ts($url, $stream_type === Plugin_Constants::MPEG);
    }

    /**
     * Get information from the account
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] | string[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

        $parse_pattern = $this->get_tv_parse_pattern();
        if (empty($parse_pattern))
            return false;

        if (isset($this->account_data) && !$force) {
            return $this->account_data;
        }

        unset($this->account_data);

        $this->ClearPlaylistCache(true);
        $this->plugin->get_m3u_parser()->setupParser($this->FetchM3U(true));
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
        hd_debug_print(null, true);
        $pl_entries = array();

        $parse_pattern = $this->get_tv_parse_pattern();
        if (empty($parse_pattern)) {
            hd_debug_print("Empty tv parsing pattern!");
        }

        $template = $this->get_current_tv_template();
        $tag_id = isset($template[Plugin_Constants::TAG_ID_MATCH]) ? $template[Plugin_Constants::TAG_ID_MATCH] : '';
        if (!empty($tag_id)) {
            hd_debug_print("ID matching tag: $tag_id");
        }

        $this->plugin->get_m3u_parser()->setupParser($this->FetchM3U(true));
        $m3u_entries = $this->get_tv_m3u_entries();
        $total = count($m3u_entries);
        hd_debug_print("Parsing $total playlist entries");

        $mapped = 0;
        foreach ($m3u_entries as $entry) {
            if (!empty($tag_id)) {
                // special case for name, otherwise take ID from selected tag
                $id = ($tag_id === 'name') ? $entry->getEntryTitle() : $entry->getEntryAttribute($tag_id);
                if (empty($id)) {
                    hd_debug_print("Unable to map ID by $tag_id for entry with url: " . $entry->getPath());
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
            hd_debug_print((string)$this->last_error);
            $this->ClearPlaylistCache(true);
        } else {
            hd_debug_print("Total entries:" . count($pl_entries) . ", mapped to ID $mapped: ");
        }

        return $pl_entries;
    }

    /**
     * @param bool $is_tv
     * @return string
     */
    protected function get_cached_playlist_name($is_tv)
    {
        if ($is_tv) {
            $tmp_file = get_temp_path($this->get_tv_list_idx() . "_playlist_tv.m3u8");
        } else {
            $tmp_file = get_temp_path($this->get_vod_list_idx() . "_playlist_vod.m3u8");
        }

        return $tmp_file;
    }

    /**
     * Clear downloaded playlist
     * @param bool $is_tv
     * @return void
     */
    public function ClearPlaylistCache($is_tv)
    {
        $tmp_file = $this->get_cached_playlist_name($is_tv);
        hd_debug_print($tmp_file, true);
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }

        if ($is_tv) {
            unset($this->tv_m3u_entries);
        } else {
            unset($this->vod_m3u_indexes);
        }
    }

    /**
     * Clear downloaded playlist
     * @return void
     */
    public function ClearChannelsCache()
    {
        $name = $this->plugin->get_parameter(PARAM_CHANNELS_LIST_NAME);
        if (empty($name)) {
            return;
        }

        $tmp_file = get_temp_path($name);
        hd_debug_print($tmp_file);
        if (file_exists($tmp_file)) {
            unlink($tmp_file);
        }
    }

    /**
     * @param array &$category_list
     * @param array &$category_index
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        $category_list = array();
        $category_index = array();

        $this->plugin->get_m3u_parser()->setupParser($this->FetchM3U(false));

        $t = microtime(1);

        $this->vod_m3u_indexes = $this->plugin->get_m3u_parser()->indexFile();
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
        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Fetched categories at " . (microtime(1) - $t) . " secs");
        HD::ShowMemoryUsage();
    }

    /**
     * @param string $keyword
     * @return array
     */
    public function getSearchList($keyword)
    {
        hd_debug_print($keyword);

        $vod_pattern = $this->get_vod_parse_pattern();
        $t = microtime(1);
        $movies = array();
        $keyword = utf8_encode(mb_strtolower($keyword, 'UTF-8'));

        foreach ($this->vod_m3u_indexes[Vod_Category::FLAG_ALL] as $index) {
            $title = $this->plugin->get_m3u_parser()->getTitleByIdx($index);
            if (empty($title)) continue;

            $search_in = utf8_encode(mb_strtolower($title, 'UTF-8'));
            if (strpos($search_in, $keyword) === false) continue;

            if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match)) {
                $title = isset($match['title']) ? $match['title'] : $title;
            }

            $entry = $this->plugin->get_m3u_parser()->getEntryByIdx($index);
            if ($entry === null) continue;

            $poster_url = $entry->getEntryAttribute('tvg-logo');
            hd_debug_print("Found at $index movie '$title', poster url: '$poster_url'");
            $movies[] = new Short_Movie($index, $title, $poster_url);
        }

        hd_debug_print("Movies found: " . count($movies));
        hd_debug_print("Search at " . (microtime(1) - $t) . " secs");

        return $movies;
    }

    /**
     * @param string $params
     * @return array
     */
    public function getFilterList($params)
    {
        //hd_debug_print("$params");
        return array();
    }

    /**
     * @param string $query_id
     * @return array
     */
    public function getMovieList($query_id)
    {
        hd_debug_print($query_id);
        $movies = array();

        $arr = explode("_", $query_id);
        $category_id = ($arr === false) ? $query_id : $arr[0];

        $current_offset = $this->get_next_page($query_id, 0);
        $indexes = $this->vod_m3u_indexes[$category_id];

        $vod_pattern = $this->get_vod_parse_pattern();
        $max = count($indexes);
        $ubound = min($max, $current_offset + 5000);
        hd_debug_print("Read from: $current_offset to $ubound");

        $pos = $current_offset;
        while($pos < $ubound) {
            $index = $indexes[$pos++];
            $entry = $this->plugin->get_m3u_parser()->getEntryByIdx($index);
            if ($entry === null || $entry->isM3U_Header()) continue;

            $title = $entry->getEntryTitle();
            if (!empty($vod_pattern) && preg_match($vod_pattern, $title, $match)) {
                $title = isset($match['title']) ? $match['title'] : $title;
            }
            $title = trim($title);

            $movies[] = new Short_Movie($index, $title, $entry->getEntryAttribute('tvg-logo'));
        }

        $this->get_next_page($query_id, $pos - $current_offset);

        return $movies;
    }

    /**
     * @param string $movie_id
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print($movie_id);
        $movie = new Movie($movie_id, $this->plugin);

        $vod_pattern = $this->get_vod_parse_pattern();
        $entry = $this->plugin->get_m3u_parser()->getEntryByIdx($movie_id);
        if ($entry === null) {
            hd_debug_print("Movie not found");
        } else {
            $logo = $entry->getEntryAttribute('tvg-logo');
            $title = $entry->getEntryTitle();
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
        }

        return $movie;
    }

    /**
     * @param string $url
     * @return string
     */
    public function UpdateVodUrlParams($url)
    {
        $vod_template = $this->get_current_vod_template();
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
     * @param int $custom_type
     * @return string
     */
    public function UpdateDuneParams($url, $custom_type = '')
    {
        $type = empty($custom_type) ? $this->get_format() : $custom_type;

        $dune_params = $this->get_stream_param($type, Stream_Params::DUNE_PARAMS);
        if (!empty($dune_params)) {
            $buf_time = $this->plugin->get_parameter(PARAM_BUFFERING_TIME, 1000);
            $dune_params = trim($dune_params, '|');
            $dune_params = str_replace(Plugin_Macros::BUFFERING, $buf_time, $dune_params);
            $url .= HD::DUNE_PARAMS_MAGIC . $dune_params;
        }

        return $url;
    }

    /**
     * @return string
     */
    public function get_format()
    {
        return $this->plugin->get_parameter(PARAM_STREAM_FORMAT, Plugin_Constants::HLS);
    }

    /**
     * @param bool $is_tv
     * @param bool $force
     * @return string
     */
    protected function FetchM3U($is_tv, $force = false)
    {
        $type = $is_tv ? "TV" : "VOD";
        $m3u_file = $this->get_cached_playlist_name($is_tv);
        hd_debug_print($m3u_file, true);
        if ($force === false) {
            if (file_exists($m3u_file)) {
                $mtime = filemtime($m3u_file);
                if (time() - $mtime > 3600) {
                    hd_debug_print("$type playlist cache expired. Forcing reload");
                    $force = true;
                } else {
                    hd_debug_print("$type playlist cache not expired");
                }
            } else {
                hd_debug_print("$type playlist not exist. Forcing reload");
                $force = true;
            }
        }

        if ($force !== false) {
            try {
                $url = $is_tv ? $this->GetPlaylistUrl() : $this->GetVodListUrl();
                if (empty($url)) {
                    hd_debug_print("$type playlist not defined");
                    throw new Exception('$type playlist not defined');
                }
                file_put_contents($m3u_file, HD::http_get_document($url));
            } catch (Exception $ex) {
                hd_debug_print("Unable to load $type playlist: " . $ex->getMessage());
            }
        }

        return $m3u_file;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @return string
     */
    protected function GetPlaylistUrl()
    {
        hd_debug_print("Get playlist url for " . $this->get_tv_template_name());
        $template = $this->get_current_tv_template();
        $url = isset($template[Plugin_Constants::PL_TEMPLATE]) ? $template[Plugin_Constants::PL_TEMPLATE] : '';
        return $this->replace_account_vars($url);
    }

    /**
     * @return string
     */
    protected function GetVodListUrl()
    {
        return $this->replace_account_vars($this->get_vod_uri());
    }

    /**
     * @param string $url
     * @return string
     */
    protected function replace_account_vars($url)
    {
        if (!empty($url)) {
            if (strpos($url, Plugin_Macros::API_URL) !== false) {
                $api_url = $this->get_feature(Plugin_Constants::PROVIDER_API_URL);
                if (empty($api_url)) {
                    hd_debug_print("Provider API url not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::API_URL, $api_url, $url);
                }
            }

            if (strpos($url, Plugin_Macros::PL_DOMAIN) !== false) {
                $domain = $this->get_domain_name();
                if (empty($domain)) {
                    hd_debug_print("Domain ID not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::PL_DOMAIN, $domain, $url);
                }
            }

            if (strpos($url, Plugin_Macros::OTT_KEY) !== false) {
                $ott_key = $this->get_ott_key();
                if (empty($ott_key)) {
                    hd_debug_print("OTT key not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::OTT_KEY, $ott_key, $url);
                }
            }

            if (strpos($url, Plugin_Macros::LOGIN) !== false) {
                $login = $this->get_login();
                if (empty($login)) {
                    hd_debug_print("Login not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::LOGIN, $login, $url);
                }
            }

            if (strpos($url, Plugin_Macros::PASSWORD) !== false) {
                $password = $this->get_password();
                if (empty($password)) {
                    hd_debug_print("Password not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::PASSWORD, $password, $url);
                }
            }

            if (strpos($url, Plugin_Macros::S_TOKEN) !== false) {
                $this->ensure_token_loaded();
                $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
                if (empty($token)) {
                    hd_debug_print("Token not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::S_TOKEN, $token, $url);
                }
            }

            if (strpos($url, Plugin_Macros::SERVER) !== false) {
                $server = $this->get_server_name();
                if (empty($server)) {
                    hd_debug_print("Server not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::SERVER, $server, $url);
                }
            }

            if (strpos($url, Plugin_Macros::SERVER_ID) !== false) {
                $server_id = $this->get_server_id();
                if (empty($server_id)) {
                    hd_debug_print("Server ID not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::SERVER_ID, $server_id, $url);
                }
            }

            if (strpos($url, Plugin_Macros::QUALITY_ID) !== false) {
                $quality = $this->get_quality_id();
                if (empty($quality)) {
                    hd_debug_print("Quality ID not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::QUALITY_ID, $quality, $url);
                }
            }

            if (strpos($url, Plugin_Macros::DEVICE_ID) !== false) {
                $device = $this->get_device_id();
                if (empty($device)) {
                    hd_debug_print("Device ID not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::DEVICE_ID, $device, $url);
                }
            }

            if (strpos($url, Plugin_Macros::PROFILE_ID) !== false) {
                $profile = $this->get_profile_id();
                if (empty($profile)) {
                    hd_debug_print("Profile ID not set, but macro was used");
                } else {
                    $url = str_replace(Plugin_Macros::PROFILE_ID, $profile, $url);
                }
            }
        }
        return $url;
    }

    /**
     * @return bool
     */
    protected function ensure_token_loaded()
    {
        return true;
    }

    /**
     * @return string
     */
    public function get_tv_template_name()
    {
        $template = $this->get_current_tv_template();
        return isset($template[Plugin_Constants::PLAYLIST_NAME]) ? $template[Plugin_Constants::PLAYLIST_NAME] : $this->plugin_info['app_type_name'];
    }

    /**
     * @return string
     */
    protected function get_tv_parse_pattern()
    {
        $template = $this->get_current_tv_template();
        $parse_pattern = isset($template[Plugin_Constants::PARSE_REGEX]) ? $template[Plugin_Constants::PARSE_REGEX] : '';
        if (!empty($parse_pattern))
            $parse_pattern = "/$parse_pattern/";

        return $parse_pattern;
    }

    /**
     * @return string
     */
    public function get_vod_uri()
    {
        $vod_template = $this->get_current_vod_template();
        return isset($vod_template[Plugin_Constants::PL_TEMPLATE]) ? $vod_template[Plugin_Constants::PL_TEMPLATE] : '';
    }

    /**
     * @return string
     */
    public function get_vod_template_name()
    {
        $vod_template = $this->get_current_vod_template();
        return isset($vod_template[Plugin_Constants::PLAYLIST_NAME]) ? $vod_template[Plugin_Constants::PLAYLIST_NAME] : $this->plugin_info['app_type_name'];
    }

    /**
     * @return string
     */
    protected function get_vod_parse_pattern()
    {
        $vod_template = $this->get_current_vod_template();
        $vod_pattern = isset($vod_template[Plugin_Constants::PARSE_REGEX]) ? $vod_template[Plugin_Constants::PARSE_REGEX] : '';
        if (!empty($vod_pattern))
            $vod_pattern = "/$vod_pattern/";

        return $vod_pattern;
    }
}

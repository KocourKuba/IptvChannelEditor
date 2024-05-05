<?php
require_once 'lib/hashed_array.php';
require_once 'lib/default_group.php';
require_once 'lib/default_channel.php';
require_once 'lib/default_epg_item.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';


class Starnet_Tv implements User_Input_Handler
{
    const ID = 'tv';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @var Starnet_Plugin
     */
    protected $plugin;

    /**
     * @var string
     */
    protected $playback_url_is_stream_url;

    /**
     * @template Channel
     * @var Hashed_Array<Channel>
     */
    protected $channels;

    /**
     * @template Group
     * @var Hashed_Array<Group>
     */
    protected $groups;

    /**
     * @template Group
     * @var Hashed_Array<string, Group>
     */
    protected $special_groups;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Starnet_Plugin $plugin
     */
    public function __construct(Starnet_Plugin $plugin)
    {
        $this->plugin = $plugin;
        $this->playback_url_is_stream_url = false;

        User_Input_Handler_Registry::get_instance()->register_handler($this);
    }

    /**
     * @inheritDoc
     */
    public static function get_handler_id()
    {
        return static::ID . '_handler';
    }

    /**
     * @return Hashed_Array<Channel>
     */
    public function get_channels()
    {
        return $this->channels;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @return Channel|null
     */
    public function get_channel($channel_id)
    {
        $channels = $this->get_channels();
        if ($channels === null) {
            hd_debug_print("Channels no loaded");
            return null;
        }

        $channel = $this->channels->get($channel_id);

        if (is_null($channel)) {
            hd_debug_print("Unknown channel: $channel_id");
        }

        return $channel;
    }

    /**
     * @param Channel $channel
     */
    public function set_channel(Channel $channel)
    {
        $this->channels->set($channel->get_id(), $channel);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @template Group
     * @return  Hashed_Array<Group>
     */
    public function get_groups()
    {
        return $this->groups;
    }

    /**
     * @param string $group_id
     * @return Group|null
     */
    public function get_group($group_id)
    {
        if (is_null($this->groups)) {
            hd_debug_print("Playlist not loaded yet. Groups not available");
            return null;
        }

        return $this->groups->get($group_id);
    }

    /**
     * @return Hashed_Array
     */
    public function get_special_groups()
    {
        return $this->special_groups;
    }

    /**
     * @param $id
     * @return Group
     */
    public function get_special_group($id)
    {
        return $this->special_groups->get($id);
    }

    /**
     * @param mixed $id
     * @param Group $group
     */
    public function set_special_group($id, $group)
    {
        $this->special_groups->set($id, $group);
    }

    public function unload_channels()
    {
        hd_debug_print(null, true);
        unset($this->channels, $this->groups, $this->special_groups);
        $this->channels = null;
        $this->groups = null;
        $this->special_groups = null;
        $this->plugin->set_need_update_epfs();
    }

    /**
     * @return int
     */
    public function load_channels()
    {
        if (isset($this->channels)) {
            return 0;
        }

        HD::ShowMemoryUsage();

        $this->plugin->get_epg_manager()->set_cache_ttl($this->plugin->get_parameter(PARAM_EPG_CACHE_TTL, 3));

        $pass_sex = $this->plugin->get_parameter(PARAM_ADULT_PASSWORD, '0000');
        $enable_protected = !empty($pass_sex);

        $this->plugin->config->GetAccountInfo();
        $pl_entries = $this->plugin->config->GetPlaylistStreamsInfo();

        $this->plugin->init_epg_manager();
        $this->plugin->unload_favorites();
        $channels_list_path = '';
        try {
            $this->plugin->config->get_channel_list($channels_list);
            $source = $this->plugin->get_parameter(PARAM_CHANNELS_SOURCE, 1);
            hd_debug_print("Load channels list using source: $source");
            switch ($source) {
                case 1:
                    $channels_list_path = smb_tree::get_folder_info($this->plugin->get_parameter(PARAM_CHANNELS_LIST_PATH, get_install_path()));
                    $channels_list_path .= $channels_list;
                    hd_debug_print("load from: $channels_list_path");
                    break;
                case 2:
                    $url_path = $this->plugin->get_parameter(PARAM_CHANNELS_URL);
                    if (empty($url_path)) {
                        $url_path = $this->plugin->config->plugin_info['app_channels_url_path'];
                    }

                    $url_path = get_slash_trailed_path($url_path) . $channels_list;
                    hd_debug_print("load folder link: $url_path");
                    break;
                case 3:
                    $url_path = $this->plugin->get_parameter(PARAM_CHANNELS_DIRECT_URL);
                    if (empty($url_path)) {
                        if (!isset($this->plugin->config->plugin_info['app_direct_links'][$channels_list])) {
                            throw new Exception(__METHOD__ . ": Direct link not set!");
                        }
                        $url_path = $this->plugin->config->plugin_info['app_direct_links'][$channels_list];
                    }

                    hd_debug_print("load direct link: $url_path");
                    break;
            }

            if (!empty($url_path)) {
                try {
                    $channels_list_path = get_data_path(hash('crc32', $url_path));
                    if (is_file($channels_list_path)) {
                        unlink($channels_list_path);
                    }
                    file_put_contents($channels_list_path, HD::http_get_document($url_path));
                } catch (Exception $ex) {
                    if (!file_exists($channels_list_path)) {
                        hd_debug_print("Can't fetch channel_list from $url_path " . $ex->getMessage());
                        return -1;
                    }
                }
            }

            file_put_contents(get_temp_path("current_list.xml"), file_get_contents($channels_list_path));
            $xml = HD::parse_xml_file($channels_list_path);
        } catch (Exception $ex) {
            hd_debug_print("Can't fetch channel_list $channels_list_path " . $ex->getMessage());
            return -1;
        }

        if ($xml->getName() !== 'tv_info') {
            $msg = "Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'";
            hd_debug_print($msg);
            $this->plugin->config->set_last_error($msg);
            return -1;
        }

        $max_support_ch_list_ver = $this->plugin->config->plugin_info['app_ch_list_version'];
        if ($max_support_ch_list_ver < (int)$xml->vesion_info->list_version) {
            $message = TR::load_string('warn_msg1');
            hd_debug_print($message);
            $this->plugin->config->set_last_error($message);
        }

        // All channels category
        $all_channels = new Default_Group(
            $this->plugin,
            ALL_CHANNEL_GROUP_ID,
            TR::load_string(Default_Group::ALL_CHANNEL_GROUP_CAPTION),
            Default_Group::ALL_CHANNEL_GROUP_ICON);

        // Favorites group
        $fav_group = new Default_Group(
            $this->plugin,
            FAVORITES_GROUP_ID,
            TR::load_string(Default_Group::FAV_CHANNEL_GROUP_CAPTION),
            Default_Group::FAV_CHANNEL_GROUP_ICON);

        // History channels category
        $history_channels = new Default_Group(
            $this->plugin,
            HISTORY_GROUP_ID,
            TR::load_string(Default_Group::HISTORY_GROUP_CAPTION),
            Default_Group::HISTORY_GROUP_ICON);

        // Vod group
        $vod_group = new Default_Group(
            $this->plugin,
            VOD_GROUP_ID,
            TR::load_string(Default_Group::VOD_GROUP_CAPTION),
            Default_Group::VOD_GROUP_ICON);

        // read category
        $fav_category_id = FAVORITES_GROUP_ID;

        // build categories in correct order
        $this->groups = new Hashed_Array();

        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = __METHOD__ . ": Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_debug_print($error_string);
                return -1;
            }

            if (isset($xml_tv_category->special_group)) {
                hd_debug_print("special group $xml_tv_category->special_group, icon: $xml_tv_category->icon_url");
                switch ((string)$xml_tv_category->special_group) {
                    case ALL_CHANNEL_GROUP_ID:
                        if (isset($xml_tv_category->icon_url)) {
                            $all_channels->set_icon_url((string)$xml_tv_category->icon_url);
                        }
                        break;
                    case FAVORITES_GROUP_ID:
                        $fav_category_id = (int)$xml_tv_category->id;
                        if (isset($xml_tv_category->icon_url)) {
                            $fav_group->set_icon_url((string)$xml_tv_category->icon_url);
                        }
                        break;
                    case HISTORY_GROUP_ID:
                        if (isset($xml_tv_category->icon_url)) {
                            $history_channels->set_icon_url((string)$xml_tv_category->icon_url);
                        }
                        break;
                    case VOD_GROUP_ID:
                        if (isset($xml_tv_category->icon_url)) {
                            $vod_group->set_icon_url((string)$xml_tv_category->icon_url);
                        }
                        break;
                }
            } else if (!isset($xml_tv_category->disabled)) {
                $group = new Default_Group(
                    $this->plugin,
                    (string)$xml_tv_category->id,
                    (string)$xml_tv_category->caption,
                    (string)$xml_tv_category->icon_url);

                $this->groups->set((string)$xml_tv_category->id, $group);
                hd_debug_print("Added category: $xml_tv_category->caption");
            }
        }

        $fav_channel_ids = $this->plugin->get_favorites();
        $fav_cnt = $fav_channel_ids->size();

        // Read channels
        $this->channels = new Hashed_Array();
        $num = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_debug_print("Error: unexpected node '{$xml_tv_channel->getName()}'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) {
                hd_debug_print("Channel $xml_tv_channel->caption is disabled");
                continue;
            }

            // Read category id from channel
            if (!isset($xml_tv_channel->tv_category_id)) {
                hd_debug_print("Error: Category undefined for channel $xml_tv_channel->caption !");
                continue;
            }

            // read channel id and custom url if exist
            if (isset($xml_tv_channel->channel_id)) {
                $channel_id = (string)$xml_tv_channel->channel_id;
                $ext_params = isset($pl_entries[$channel_id]) ? $pl_entries[$channel_id] : array();
                // update stream url by channel ID
                $hash = $channel_id;
                $streaming_url = isset($xml_tv_channel->streaming_url) ? (string)$xml_tv_channel->streaming_url : '';
            } else {
                // custom url, no id, play as is set channel id as url hash
                $streaming_url = (string)$xml_tv_channel->streaming_url;
                $hash = $channel_id = hash('crc32', $streaming_url);
                $ext_params = array();
            }

            $streaming_url_type = (!empty($streaming_url) && isset($xml_tv_channel->custom_url_type)) ? (string)$xml_tv_channel->custom_url_type : '';

            // custom archive url or template
            $custom_archive_url = isset($xml_tv_channel->catchup_url_template) ? (string)$xml_tv_channel->catchup_url_template : '';
            $custom_arc_url_type = isset($xml_tv_channel->custom_arc_url_type) ? (string)$xml_tv_channel->custom_arc_url_type : '';

            $num++;
            $tv_category_id = (int)$xml_tv_channel->tv_category_id;
            if ($this->channels->has($hash)) {
                $channel = $this->channels->get($hash);
                if ($tv_category_id !== $fav_category_id) {
                    foreach($channel->get_groups() as $group) {
                        if ($group->get_id() !== $fav_category_id) {
                            hd_debug_print("Channel $xml_tv_channel->caption ($channel_id) already exist in category: {$group->get_title()} ({$group->get_id()})");
                        }
                    }
                }
            } else {
                $icon_url = (string)$xml_tv_channel->icon_url;
                $number = $num;

                $epg_ids = array();
                $epg_ids[] = (string)$xml_tv_channel->epg_id;
                if (!empty($xml_tv_channel->tvg_id)) {
                    $epg_ids[] = (string)$xml_tv_channel->tvg_id;
                }
                $epg_ids[] = $channel_id;
                $epg_ids[] = (string)$xml_tv_channel->caption;
                $epg_ids = array_unique($epg_ids);

                $protected = (int)$xml_tv_channel->protected && $enable_protected;
                $channel = new Default_Channel(
                    $hash,
                    $channel_id,
                    (string)$xml_tv_channel->caption,
                    $icon_url,
                    $streaming_url,
                    $custom_archive_url,
                    $streaming_url_type,
                    $custom_arc_url_type,
                    (int)$xml_tv_channel->archive,
                    $number,
                    $epg_ids,
                    $protected,
                    (int)$xml_tv_channel->timeshift_hours,
                    $ext_params
                );

                $this->channels->set($channel->get_id(), $channel);
            }

            // Link group and channel.
            if (($tv_category_id === $fav_category_id || isset($xml_tv_channel->favorite))) {
                // favorites category
                if (!$fav_channel_ids->in_order($channel_id)) {
                    hd_debug_print("Added from channels list to favorites channel $hash ($xml_tv_channel->caption)");
                    $fav_channel_ids->add_item($channel_id);
                }
            } else if (!$this->groups->has($tv_category_id)) {
                // Category disabled or unknown
                hd_debug_print("Unknown category $tv_category_id");
            } else {
                $group = $this->groups->get($tv_category_id);
                if (is_null($group))
                    hd_debug_print("unknown group: $tv_category_id");
                $channel->add_group($group);
                $group->add_channel($channel);
            }
        }

        if ($fav_cnt !== $fav_channel_ids->size()) {
            $this->plugin->save_favorites();
        }

        $this->special_groups = new Hashed_Array();
        $this->special_groups->set($fav_group->get_id(), $fav_group);
        $this->special_groups->set($history_channels->get_id(), $history_channels);
        $this->special_groups->set($all_channels->get_id(), $all_channels);
        $this->special_groups->set($vod_group->get_id(), $vod_group);

        if (empty($pl_entries)
            && $this->plugin->config->plugin_info['app_type_name'] !== 'custom'
            && $this->channels->size()) {
            $message = TR::load_string('warn_msg6');
            $this->plugin->config->set_last_error($message);
            hd_debug_print($message);
            $this->plugin->config->ClearPlaylistCache(true);
        }

        hd_debug_print("Loaded: channels: {$this->channels->size()}, groups: {$this->groups->size()}");
        HD::ShowMemoryUsage();

        if ($this->plugin->get_parameter(PARAM_EPG_CACHE_ENGINE) === ENGINE_XMLTV) {
            $this->plugin->start_bg_indexing();
            sleep(1);
        }

        return 1;
    }

    /**
     * @return int
     */
    public function reload_channels()
    {
        hd_debug_print("Reload channels");
        $this->plugin->config->ClearPlaylistCache(true);
        $this->plugin->config->ClearChannelsCache();

        $this->unload_channels();
        return $this->load_channels();
    }

    /**
     * @param string $channel_id
     * @param int $archive_ts
     * @param string $protect_code
     * @return string
     */
    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code)
    {
        hd_debug_print("channel: $channel_id archive_ts: $archive_ts, protect code: $protect_code");

        try {
            if ($this->load_channels() === -1) {
                throw new Exception("Channels not loaded!");
            }

            $pass_sex = ($this->plugin->get_parameter(PARAM_ADULT_PASSWORD, '0000'));
            // get channel by hash
            $channel = $this->get_channel($channel_id);
            if ($channel === null) {
                throw new Exception("Undefined channel!");
            }

            if ($channel->is_protected()) {
                if ($protect_code !== $pass_sex) {
                    throw new Exception("Wrong adult password");
                }
            } else {
                $now = $channel->get_archive() > 0 ? time() : 0;
                $this->plugin->get_playback_points()->push_point($channel_id, ($archive_ts !== -1 ? $archive_ts : $now));
            }

            // update url if play archive or different type of the stream
            $url = $this->plugin->config->GenerateStreamUrl($channel, $archive_ts);

            if ($this->plugin->get_bool_parameter(PARAM_PER_CHANNELS_ZOOM)) {
                $zoom_preset = $this->plugin->get_channel_zoom($channel_id);
                if (!is_null($zoom_preset)) {
                    if (!is_android()) {
                        $zoom_preset = DuneVideoZoomPresets::normal;
                        hd_debug_print("zoom_preset: reset to normal $zoom_preset");
                    }

                    if ($zoom_preset !== DuneVideoZoomPresets::not_set) {
                        $url .= (strpos($url, HD::DUNE_PARAMS_MAGIC) === false ? HD::DUNE_PARAMS_MAGIC : ",");
                        $url .= "zoom:$zoom_preset";
                    }
                }
            }

            hd_debug_print($url);
        } catch (Exception $ex) {
            hd_debug_print("Exception: " . $ex->getMessage());
            $url = '';
        }

        return $url;
    }

    /**
     * @param MediaURL $media_url
     * @return array
     */
    public function get_tv_info(MediaURL $media_url)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url->get_media_url_str(), true);

        if ($this->load_channels() === -1) {
            hd_debug_print("Channels not loaded!");
            return array();
        }

        $group_all = $this->get_special_group(ALL_CHANNEL_GROUP_ID);
        $show_all = !$group_all->is_disabled();
        $all_channels = new Hashed_Array();
        $all_groups_ids = array();
        /** @var Group $group */
        foreach ($this->groups as $group) {
            if ($group->is_disabled()) continue;

            $all_groups_ids[] = $group->get_id();
            /** @var Group $group */
            foreach ($group->get_group_channels() as $channel) {
                if (is_null($channel)) continue;

                $group_id_arr = new Hashed_Array();
                if ($show_all) {
                    $group_id_arr->put(ALL_CHANNEL_GROUP_ID, '');
                }

                foreach ($channel->get_groups() as $in_group) {
                    $group_id_arr->put($in_group->get_id(), '');
                }
                if ($group_id_arr->size() === 0) continue;

                $all_channels->put(
                    $channel->get_id(),
                    array(
                        PluginTvChannel::id => $channel->get_id(),
                        PluginTvChannel::caption => $channel->get_title(),
                        PluginTvChannel::group_ids => $group_id_arr->get_keys(),
                        PluginTvChannel::icon_url => $channel->get_icon_url(),
                        PluginTvChannel::number => $channel->get_number(),

                        PluginTvChannel::have_archive => $channel->get_archive() > 0,
                        PluginTvChannel::is_protected => $channel->is_protected(),

                        // set default epg range
                        PluginTvChannel::past_epg_days => $channel->get_past_epg_days(),
                        PluginTvChannel::future_epg_days => $channel->get_future_epg_days(),

                        PluginTvChannel::archive_past_sec => $channel->get_archive_past_sec(),
                        PluginTvChannel::archive_delay_sec => $this->plugin->get_parameter(PARAM_ARCHIVE_DELAY_TIME, 60),

                        // Buffering time
                        PluginTvChannel::buffering_ms => $this->plugin->get_parameter(PARAM_BUFFERING_TIME, 1000),
                        PluginTvChannel::timeshift_hours => $channel->get_timeshift_hours(),

                        PluginTvChannel::playback_url_is_stream_url => $this->playback_url_is_stream_url,
                    )
                );
            }
        }

        $groups = array();

        if ($show_all) {
            $group = $this->get_special_group(ALL_CHANNEL_GROUP_ID);
            if ($group !== null) {
                $groups[] = array(
                    PluginTvGroup::id => $group->get_id(),
                    PluginTvGroup::caption => $group->get_title(),
                    PluginTvGroup::icon_url => $group->get_icon_url()
                );
            }
        }

        /** @var Group $group */
        foreach ($all_groups_ids as $id) {
            $group = $this->get_group($id);
            if (is_null($group)) continue;

            $groups[] = array(
                PluginTvGroup::id => $group->get_id(),
                PluginTvGroup::caption => $group->get_title(),
                PluginTvGroup::icon_url => $group->get_icon_url()
            );
        }

        if ((string)$media_url->group_id === FAVORITES_GROUP_ID) {
            $initial_group_id = null;
            $initial_is_favorite = 1;
        } else {
            $initial_group_id = (string)$media_url->group_id;
            $initial_is_favorite = 0;
        }

        return array(
            PluginTvInfo::show_group_channels_only => true,

            PluginTvInfo::groups => $groups,
            PluginTvInfo::channels => $all_channels->get_ordered_values(),

            PluginTvInfo::favorites_supported => true,
            PluginTvInfo::favorites_icon_url => $this->get_special_group(FAVORITES_GROUP_ID)->get_icon_url(),

            PluginTvInfo::initial_channel_id => (string)$media_url->channel_id,
            PluginTvInfo::initial_group_id => $initial_group_id,

            PluginTvInfo::initial_is_favorite => $initial_is_favorite,
            PluginTvInfo::favorite_channel_ids => $this->plugin->get_favorites()->get_order(),

            PluginTvInfo::initial_archive_tm => isset($media_url->archive_tm) ? (int)$media_url->archive_tm : -1,

            PluginTvInfo::epg_font_size => $this->plugin->get_bool_parameter(PARAM_EPG_FONT_SIZE, false) ? PLUGIN_FONT_SMALL : PLUGIN_FONT_NORMAL,

            PluginTvInfo::actions => $this->get_action_map(),
            PluginTvInfo::timer => Action_Factory::timer(1000),
        );
    }

    public function get_action_map()
    {
        hd_debug_print(null, true);

        return array(
            GUI_EVENT_PLAYBACK_STOP => User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP),
            GUI_EVENT_TIMER => User_Input_Handler_Registry::create_action($this,
                GUI_EVENT_TIMER,
                null,
                $this->plugin->get_epg_manager()->is_index_locked() ? array('locked' => true) : null),
        );
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        if (!isset($user_input->control_id))
            return null;

        switch ($user_input->control_id) {
            case GUI_EVENT_TIMER:
                $post_action = null;

                if (isset($user_input->stop_play)) {
                    // rising after playback end + 100 ms
                    $this->plugin->set_need_update_epfs();
                    $post_action = $this->plugin->invalidate_epfs_folders($plugin_cookies, array(Starnet_TV_History_Screen::ID));
                } else if (isset($user_input->locked)) {
                    clearstatcache();
                    if ($this->plugin->get_epg_manager()->is_index_locked()) {
                        $new_actions = $this->get_action_map();
                        $new_actions[GUI_EVENT_TIMER] = User_Input_Handler_Registry::create_action($this,
                            GUI_EVENT_TIMER,
                            null,
                            array('locked' => true));

                        hd_debug_print("EPG still indexed...");
                        $post_action = Action_Factory::change_behaviour($new_actions, 5000);
                    } else {
                        hd_debug_print("Index EPG done");
                        foreach($this->plugin->get_epg_manager()->get_delayed_epg() as $channel_id) {
                            hd_debug_print("Refresh EPG for channel ID: $channel_id");
                            $day_start_ts = strtotime(date("Y-m-d")) + get_local_time_zone_offset();
                            $day_epg = $this->plugin->get_day_epg($channel_id, $day_start_ts, $plugin_cookies);
                            $post_action = Action_Factory::update_epg($channel_id, true, $day_start_ts, $day_epg, $post_action);
                        }
                        $this->plugin->get_epg_manager()->clear_delayed_epg();
                    }
                }

                return $post_action;

            case GUI_EVENT_PLAYBACK_STOP:
                $this->plugin->get_playback_points()->update_point($user_input->plugin_tv_channel_id);

                if (!isset($user_input->playback_stop_pressed) && !isset($user_input->playback_power_off_needed)) {
                    return null;
                }

                $this->plugin->get_playback_points()->save();
                $new_actions = $this->get_action_map();
                $new_actions[GUI_EVENT_TIMER] = User_Input_Handler_Registry::create_action($this,
                    GUI_EVENT_TIMER,
                    null,
                    array('stop_play' => true));

                return Action_Factory::change_behaviour($new_actions, 100);
        }

        return null;
    }
}

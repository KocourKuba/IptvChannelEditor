<?php
require_once 'lib/tv/abstract_tv.php';
require_once 'lib/tv/default_channel.php';
require_once 'lib/tv/default_epg_item.php';
require_once 'lib/hashed_array.php';
require_once 'lib/epg_xml_parser.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';


class Starnet_Tv extends Abstract_Tv implements User_Input_Handler
{
    const ID = 'tv_handler';

    /**
     * @var Starnet_Plugin
     */
    protected $plugin;

    /**
     * @param Starnet_Plugin $plugin
     */
    public function __construct(Starnet_Plugin $plugin)
    {
        $this->plugin = $plugin;
        parent::__construct(Abstract_Tv::MODE_CHANNELS_N_TO_M, false);

        User_Input_Handler_Registry::get_instance()->register_handler($this);
    }

    public function get_handler_id()
    {
        return self::ID;
    }

    /**
     * @return bool
     */
    public function is_favorites_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::TV_FAVORITES_SUPPORTED);
    }

    /**
     * @param array &$items
     */
    public function add_special_groups(&$items)
    {
        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url =>
                    MediaURL::encode(
                        array
                        (
                            'screen_id' => Starnet_Vod_Category_List_Screen::ID,
                            'name' => 'VOD',
                        )),
                PluginRegularFolderItem::caption => Default_Dune_Plugin::VOD_GROUP_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Dune_Plugin::VOD_GROUP_ICON
                )
            );
        }
    }

    /**
     * @param $plugin_cookies
     * @throws Exception
     */
    public function load_channels(&$plugin_cookies)
    {
        $channels_list_path = '';
        try {
            $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
            $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;
            switch ($source) {
                case 1:
                    $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path') . $channels_list;
                    break;
                case 2:
                    if (isset($plugin_cookies->channels_url) && !empty($plugin_cookies->channels_url)) {
                        $url_path = $plugin_cookies->channels_url;
                    } else {
                        $url_path = $this->plugin->plugin_info['app_channels_url_path'];
                    }

                    $channels_list_path = get_temp_path($channels_list);
                    if (!file_exists($channels_list_path)) {
                        file_put_contents($channels_list_path, HD::http_get_document($url_path . $channels_list));
                    }
                    break;
            }

            $xml = HD::parse_xml_file($channels_list_path);
        } catch (Exception $ex) {
            hd_print("Can't fetch channel_list $channels_list_path");
            return;
        }

        if ($xml->getName() !== 'tv_info') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'tv_info'");
            throw new Exception('Invalid XML document');
        }

        // Create channels and groups
        $this->channels = new Hashed_Array();
        $this->groups = new Hashed_Array();

        // Favorites group
        if ($this->is_favorites_supported()) {
            $this->groups->put(new Favorites_Group(
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_ICON_PATH));
            $fav_ids = $this->get_fav_channel_ids($plugin_cookies);
            hd_print("Favorites loaded from settings: " . count($fav_ids));
        }

        // All channels group
        $this->groups->put(new All_Channels_Group(
            $this,
            Default_Dune_Plugin::ALL_CHANNEL_GROUP_CAPTION,
            Default_Dune_Plugin::ALL_CHANNEL_GROUP_ICON_PATH));

        // read category
        $fav_category_id = Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID;
        foreach ($xml->tv_categories->children() as $xml_tv_category) {
            if ($xml_tv_category->getName() !== 'tv_category') {
                $error_string = "Error: unexpected node '{$xml_tv_category->getName()}'. Expected: 'tv_category'";
                hd_print($error_string);
                throw new Exception($error_string);
            }

            if (isset($xml_tv_category->disabled)) {
                continue;
            }

            if (!isset($xml_tv_category->favorite)) {
                $this->groups->put(new Default_Group((string)$xml_tv_category->id,
                    (string)$xml_tv_category->caption,
                    (string)$xml_tv_category->icon_url));
                hd_print("Added category: $xml_tv_category->caption");
            } else if ($xml_tv_category->favorite == "true") {
                $fav_category_id = (int)$xml_tv_category->id;
                hd_print("Embedded Favorites category: $xml_tv_category->caption ($fav_category_id)");
            }
        }

        $this->plugin->config->GetAccountInfo($plugin_cookies);
        $pl_entries = $this->plugin->config->GetPlaylistStreamsInfo($plugin_cookies);

        $fav_channel_ids = $this->get_fav_channel_ids($plugin_cookies);

        // Read channels
        $num = 0;
        foreach ($xml->tv_channels->children() as $xml_tv_channel) {
            if ($xml_tv_channel->getName() !== 'tv_channel') {
                hd_print("Error: unexpected node '{$xml_tv_channel->getName()}'. Expected: 'tv_channel'");
                continue;
            }

            // ignore disabled channel
            if (isset($xml_tv_channel->disabled)) {
                hd_print("Channel $xml_tv_channel->caption is disabled");
                continue;
            }

            // Read category id from channel
            if (!isset($xml_tv_channel->tv_category_id)) {
                hd_print("Error: Category undefined for channel $xml_tv_channel->caption !");
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

            $streaming_url_type = (!empty($streaming_url) && isset($xml_tv_channel->custom_url_type)) ? $xml_tv_channel->custom_url_type : '';

            // custom archive url or template
            $custom_archive_url = isset($xml_tv_channel->catchup_url_template) ? $xml_tv_channel->catchup_url_template : '';
            $custom_arc_url_type = isset($xml_tv_channel->custom_arc_url_type) ? $xml_tv_channel->custom_arc_url_type : '';

            $num++;
            $tv_category_id = (int)$xml_tv_channel->tv_category_id;
            if ($this->channels->has($hash)) {
                $channel = $this->channels->get($hash);
                if ($tv_category_id !== $fav_category_id) {
                    hd_print("$tv_category_id !== $fav_category_id");
                    hd_print("Channel $xml_tv_channel->caption ($channel_id) already exist in category $tv_category_id");
                }
            } else {
                // https not supported for old players
                // $icon_url = str_replace("https://", "http://", (string)$xml_tv_channel->icon_url);
                $icon_url = (string)$xml_tv_channel->icon_url;
                $number = isset($xml_tv_channel->int_id) ? (int)$xml_tv_channel->int_id : $num;

                $epg1 = (string)$xml_tv_channel->epg_id;
                $epg2 = (empty($xml_tv_channel->tvg_id)) ? $epg1 : (string)$xml_tv_channel->tvg_id;

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
                    $epg1,
                    $epg2,
                    (int)$xml_tv_channel->protected,
                    (int)$xml_tv_channel->timeshift_hours,
                    $ext_params
                );
                $this->channels->put($channel);
            }

            // Link group and channel.
            if (($tv_category_id === $fav_category_id || $xml_tv_channel->favorite) && $this->is_favorites_supported()) {
                // favorites category
                if (in_array($channel_id, $fav_channel_ids) === false) {
                    hd_print("Added from channels list to favorites channel $hash ($xml_tv_channel->caption)");
                    $fav_channel_ids[] = $channel_id;
                }
            } else if (!$this->groups->has($tv_category_id)) {
                // Category disabled or unknown
                hd_print("Unknown category $tv_category_id");
            } else {
                $group = $this->groups->get($tv_category_id);
                if (is_null($group))
                    hd_print("unknown group: $tv_category_id");
                $channel->add_group($group);
                $group->add_channel($channel);
            }
        }

        $this->set_fav_channel_ids($plugin_cookies, $fav_channel_ids);

        hd_print("Loaded: channels: {$this->channels->size()}, groups: {$this->groups->size()}");
    }

    /**
     * @param string $playback_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_stream_url($playback_url, &$plugin_cookies)
    {
        return $playback_url;
    }

    /**
     * @param string $channel_id
     * @param int $archive_ts
     * @param string $protect_code
     * @param $plugin_cookies
     * @return string
     */
    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies)
    {
        Playback_Points::update();

        try {
            $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
            // get channel by hash
            $channel = $this->get_channel($channel_id);
            if ($protect_code !== $pass_sex && $channel->is_protected()) {
                hd_print('Wrong adult password');
                throw new Exception('Wrong adult password');
            }
        } catch (Exception $ex) {
            hd_print("get_tv_playback_url: Exception " . $ex->getMessage());
            return '';
        }

        if (empty($protect_code)) {
            Playback_Points::push($channel_id, $archive_ts);
        }

        // update url if play archive or different type of the stream
        $url = $this->plugin->config->GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
        hd_print("get_tv_playback_url: $url");
        return $url;
    }

    /**
     * @param string $channel_id
     * @param integer $day_start_ts
     * @param $plugin_cookies
     * @return array
     */
    public function get_day_epg($channel_id, $day_start_ts, &$plugin_cookies)
    {
        try {
            // get channel by hash
            $channel = $this->get_channel($channel_id);
        } catch (Exception $ex) {
            hd_print("Can't get channel with ID: $channel_id");
            return array();
        }

        $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;
        $day_epg = $channel->get_day_epg_items($epg_source, $day_start_ts);
        if ($day_epg !== false)
            return $day_epg;


        $epg_man = new Epg_Manager($this->plugin->config);

        try {
            $day_start_ts -= get_local_time_zone_offset();
            $epg = $epg_man->get_epg($channel, $epg_source, $day_start_ts, $plugin_cookies);
            if (count($epg) === 0) {
                hd_print("No data from $epg_source EPG");
                return array();
            }
        } catch (Exception $ex) {
            hd_print("Can't fetch EPG ID from $epg_source epg source: " . $ex->getMessage());
            return array();
        }

        hd_print("Loaded " . count($epg) . " EPG entries");
        // get personal time shift for channel
        $time_shift = $channel->get_timeshift_hours() * 3600;
        $day_epg = array();
        foreach ($epg as $time => $value) {
            $day_epg[] = array
            (
                PluginTvEpgProgram::start_tm_sec => (int)$time + $time_shift,
                PluginTvEpgProgram::end_tm_sec => (int)$value[Epg_Params::EPG_END] + $time_shift,
                PluginTvEpgProgram::name => $value[Epg_Params::EPG_NAME],
                PluginTvEpgProgram::description => $value[Epg_Params::EPG_DESC],
                PluginTvEpgProgram::icon_url => '',
                Ext_Epg_Program::year => '',
                Ext_Epg_Program::main_category => '',
            );
        }

        $channel->set_day_epg_items($epg_source, $day_start_ts, $day_epg);
        $this->set_channel($channel);

        return $day_epg;
    }

    public function get_program_info($channel_id, $program_ts, $plugin_cookies)
    {
        $day_ts = strtotime(date('d-M-Y', (isset($program_ts) ? $program_ts : time())));
        $day_epg = $this->get_day_epg($channel_id, $day_ts, $plugin_cookies);
        foreach ($day_epg as $item) {
            if ($program_ts >= $item[PluginTvEpgProgram::start_tm_sec] && $program_ts < $item[PluginTvEpgProgram::end_tm_sec]) {
                return $item;
            }
        }

        return null;
    }

    public function get_action_map()
    {
        User_Input_Handler_Registry::get_instance()->register_handler($this);
        $actions[GUI_EVENT_TIMER] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_TIMER);
        if (NEWGUI_FEAUTURES_AVAILABLE) {
            $actions[GUI_EVENT_PLAYBACK_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP);
        }

        return $actions;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Starnet_Tv: handle_user_input:');
        foreach ($user_input as $key => $value) hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case GUI_EVENT_PLAYBACK_STOP:
                if (isset($user_input->playback_stop_pressed) || isset($user_input->playback_power_off_needed)) {
                    if (NEWGUI_FEAUTURES_AVAILABLE) {
                        Playback_Points::update();
                        Starnet_Epfs_Handler::refresh_tv_epfs($plugin_cookies);

                        return Starnet_Epfs_Handler::invalidate_folders();
                    }
                }

                return null;

            case GUI_EVENT_TIMER:
                $actions = $this->get_action_map();
                return Action_Factory::change_behaviour($actions, 5000);
        }

        return null;
    }
}

<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Tv_Channel_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_channel_list';

    /**
     * @param string $group_id
     * @return false|string
     */
    public static function get_media_url_str($group_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'group_id' => $group_id,
            ));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_TV_CHANNEL_LIST_FOLDER_VIEWS());

        $plugin->create_screen($this);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("TvChannelListScreen get_action_map: " . $media_url->get_raw_string());

        $action_play = Action_Factory::tv_play();
        $action_settings = User_Input_Handler_Registry::create_action($this, ACTION_SETTINGS);

        $actions = array(
            GUI_EVENT_KEY_ENTER   => $action_play,
            GUI_EVENT_KEY_PLAY    => $action_play,
            GUI_EVENT_KEY_B_GREEN => $action_settings,
            GUI_EVENT_KEY_SETUP   => $action_settings,
        );

        if ((string)$media_url->group_id === Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID) {
            $search_action = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, 'Поиск');
            $actions[GUI_EVENT_KEY_C_YELLOW] = $search_action;
            $actions[GUI_EVENT_KEY_SEARCH] = $search_action;
        }

        if ($this->plugin->tv->is_favorites_supported()) {
            $actions[GUI_EVENT_KEY_D_BLUE] = User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, 'В избранное');
        }

        if (HD::rows_api_support()) {
            $actions[GUI_EVENT_PLAYBACK_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP);
        }

        return $actions;
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Starnet_Tv_Channel_List_Screen: handle_user_input:');
        foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $channel_id = $media_url->channel_id;
        $channel = $this->plugin->tv->get_channels()->get($channel_id);

        switch ($user_input->control_id) {
            case ACTION_ADD_FAV:
                $opt_type = $this->plugin->tv->is_favorite_channel_id($channel_id, $plugin_cookies) ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $this->plugin->tv->change_tv_favorites($opt_type, $channel_id, $plugin_cookies);
                return Action_Factory::invalidate_folders(array(self::get_media_url_str($media_url->group_id)));

            case ACTION_SETTINGS:
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');

            case ACTION_CREATE_SEARCH:
                $defs = array();
                Control_Factory::add_text_field($defs, $this, null, ACTION_NEW_SEARCH, '',
                    $channel->get_title(), false, false, true, true, 1300, false, true);
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog('Введите название или часть названия канала для поиска', $defs, true, 1300);

            case ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(User_Input_Handler_Registry::create_action($this, ACTION_RUN_SEARCH));

            case ACTION_RUN_SEARCH:
                $defs = array();
                $find_text = $user_input->{ACTION_NEW_SEARCH};
                $q = false;
                $group = $this->plugin->tv->get_group($media_url->group_id);
                foreach ($group->get_group_channels() as $idx => $tv_channel) {
                    $ch_title = $tv_channel->get_title();
                    $s = mb_stripos($ch_title, $find_text, 0, "UTF-8");
                    if ($s !== false) {
                        $q = true;
                        hd_print("found channel: $ch_title, idx: " . $idx);
                        $add_params['number'] = $idx;
                        Control_Factory::add_close_dialog_and_apply_button_title($defs, $this, $add_params,
                            ACTION_JUMP_TO_CHANNEL, '', $ch_title, 900);
                    }
                }

                if ($q === false) {
                    Control_Factory::add_multiline_label($defs, '', 'Ничего не найдено.', 6);
                    Control_Factory::add_vgap($defs, 20);
                    Control_Factory::add_close_dialog_and_apply_button_title($defs, $this, null,
                        ACTION_CREATE_SEARCH, '', 'Новый поиск', 300);
                }

                return Action_Factory::show_dialog('Поиск', $defs, true);

            case ACTION_JUMP_TO_CHANNEL:
                $ndx = (int)$user_input->number;
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $parent_media_url->group_id = $media_url->group_id;
                $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true, $ndx);

            case GUI_EVENT_PLAYBACK_STOP:
                if (isset($user_input->playback_stop_pressed) || isset($user_input->playback_power_off_needed)) {

                    if ($this->plugin->history_support && isset($user_input->plugin_tv_channel_id)) {
                        Playback_Points::update($user_input->plugin_tv_channel_id);
                    }

                    Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                    return Starnet_Epfs_Handler::invalidate_folders();
                }
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Group $group
     * @param Channel $channel
     * @param $plugin_cookies
     * @return array
     */
    private function get_regular_folder_item($group, $channel, &$plugin_cookies)
    {
        return array
        (
            PluginRegularFolderItem::media_url => MediaURL::encode(array('channel_id' => $channel->get_id(), 'group_id' => $group->get_id())),
            PluginRegularFolderItem::caption => $channel->get_title(),
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => $channel->get_icon_url(),
                ViewItemParams::item_detailed_icon_path => $channel->get_icon_url(),
            ),
            PluginRegularFolderItem::starred => $this->plugin->tv->is_favorite_channel_id($channel->get_id(), $plugin_cookies),
        );
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        try {
            $this->plugin->tv->ensure_channels_loaded($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Ошибка загрузки плейлиста! " . $e->getMessage());
            return array();
        }

        $group = $this->plugin->tv->get_group($media_url->group_id);

        $items = array();

        foreach ($group->get_group_channels() as $c) {
            $items[] = $this->get_regular_folder_item($group, $c, $plugin_cookies);
        }

        return $items;
    }
}

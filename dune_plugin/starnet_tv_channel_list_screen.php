<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Tv_Channel_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_channel_list';

    const ACTION_NEW_SEARCH = 'new_search';
    const ACTION_CREATE_SEARCH = 'create_search';
    const ACTION_RUN_SEARCH = 'run_search';
    const ACTION_JUMP_TO_CHANNEL = 'jump_to_channel';

    /**
     * @param string $group_id
     * @return false|string
     */
    public static function get_media_url_string($group_id)
    {
        return MediaURL::encode(array('screen_id' => static::ID, 'group_id' => $group_id));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_debug_print("" . $media_url->get_raw_string());

        $action_play = User_Input_Handler_Registry::create_action($this, ACTION_PLAY_ITEM);
        $search_action = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, TR::t('search'));

        $actions = array(
            GUI_EVENT_KEY_ENTER      => $action_play,
            GUI_EVENT_KEY_PLAY       => $action_play,
            GUI_EVENT_KEY_POPUP_MENU => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_KEY_INFO       => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_INFO),
            GUI_EVENT_KEY_SETUP      => User_Input_Handler_Registry::create_action($this, ACTION_SETTINGS),
            GUI_EVENT_KEY_D_BLUE     => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
        );

        $group_id = (string)$media_url->group_id;
        if ($group_id !== FAVORITES_GROUP_ID && $group_id !== HISTORY_GROUP_ID) {
            $actions[GUI_EVENT_KEY_C_YELLOW] = $search_action;
            $actions[GUI_EVENT_KEY_SEARCH] = $search_action;
        }

        return $actions;
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $selected_media_url = MediaURL::decode($user_input->selected_media_url);
        $channel_id = $selected_media_url->channel_id;
        $channel = $this->plugin->tv->get_channel($channel_id);
        if ($channel === null) {
            hd_debug_print("Undefined channel!");
            return null;
        }

        switch ($user_input->control_id) {
            case ACTION_PLAY_ITEM:
                try {
                    $post_action = $this->plugin->tv_player_exec($selected_media_url);
                } catch (Exception $ex) {
                    hd_debug_print("Channel can't played");
                    print_backtrace_exception($ex);
                    return Action_Factory::show_title_dialog(TR::t('err_channel_cant_start'),
                        null,
                        TR::t('warn_msg2__1', $ex->getMessage()));
                }

                return $this->plugin->invalidate_epfs_folders($plugin_cookies, null, $post_action);

            case ACTION_ADD_FAV:
                $opt_type = $this->plugin->get_favorites()->in_order($channel_id) ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $action = $this->plugin->change_tv_favorites($opt_type, $channel_id);
                $this->plugin->save_favorites();
                return $this->plugin->update_invalidate_epfs_folders($plugin_cookies, $action, $user_input->parent_media_url);

            case ACTION_SETTINGS:
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), TR::t('entry_setup'));

            case ACTION_CREATE_SEARCH:
                $defs = array();
                Control_Factory::add_text_field($defs, $this, null, ACTION_NEW_SEARCH, '',
                    $channel->get_title(), false, false, true, true, 1300, false, true);
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog(TR::t('tv_screen_search_channel'), $defs, true, 1300);

            case ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(User_Input_Handler_Registry::create_action($this, ACTION_RUN_SEARCH));

            case ACTION_RUN_SEARCH:
                $find_text = $user_input->{self::ACTION_NEW_SEARCH};
                hd_debug_print("Search in group: $parent_media_url->group_id", true);
                $parent_group = $parent_media_url->group_id === ALL_CHANNEL_GROUP_ID
                    ? $this->plugin->tv->get_special_group($parent_media_url->group_id)
                    : $this->plugin->tv->get_group($parent_media_url->group_id);

                if (is_null($parent_group)) {
                    hd_debug_print("unknown parent group", true);
                    break;
                }

                return $this->do_search($parent_group, $find_text);

            case self::ACTION_JUMP_TO_CHANNEL:
                return $this->invalidate_current_folder($parent_media_url, $plugin_cookies, $user_input->number);

            case GUI_EVENT_KEY_POPUP_MENU:
                $menu_items = array();

                if (!is_limited_apk()) {
                    $is_external = $this->plugin->is_channel_for_ext_player($channel_id);
                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_EXTERNAL_PLAYER,
                        TR::t('tv_screen_external_player'),
                        ($is_external ? "play.png" : null)
                    );

                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_INTERNAL_PLAYER,
                        TR::t('tv_screen_internal_player'),
                        ($is_external ? null : "play.png")
                    );

                    $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);
                }

                if ($this->plugin->get_bool_parameter(PARAM_PER_CHANNELS_ZOOM)) {
                    $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ZOOM_POPUP_MENU, TR::t('video_aspect_ratio'), "aspect.png");
                }

                $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);

                $menu_items[] = $this->plugin->create_menu_item($this, GUI_EVENT_KEY_INFO, TR::t('channel_info_dlg'), "info.png");

                return Action_Factory::show_popup_menu($menu_items);

            case ACTION_ZOOM_POPUP_MENU:
                $menu_items = array();
                $zoom_data = $this->plugin->get_channel_zoom($selected_media_url->channel_id);
                foreach (DuneVideoZoomPresets::$zoom_ops as $idx => $zoom_item) {
                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_ZOOM_APPLY,
                        TR::t($zoom_item),
                        (strcmp($idx, $zoom_data) !== 0 ? null : "check.png"),
                        array(ACTION_ZOOM_SELECT => (string)$idx));
                }

                return Action_Factory::show_popup_menu($menu_items);
            case ACTION_ZOOM_APPLY:
                if (isset($user_input->{ACTION_ZOOM_SELECT})) {
                    $zoom_select = $user_input->{ACTION_ZOOM_SELECT};
                    $this->plugin->set_channel_zoom($channel_id, ($zoom_select !== DuneVideoZoomPresets::not_set) ? $zoom_select : null);
                }
                break;

            case ACTION_EXTERNAL_PLAYER:
            case ACTION_INTERNAL_PLAYER:
                $this->plugin->set_channel_for_ext_player($channel_id, $user_input->control_id === ACTION_EXTERNAL_PLAYER);
                break;

            case GUI_EVENT_KEY_INFO:
                return $this->plugin->do_show_channel_info($channel_id);
        }

        return null;
    }

    /**
     * @param Group $parent_group
     * @param string $find_text
     * @return array
     */
    protected function do_search(Group $parent_group, $find_text)
    {
        hd_debug_print("Find text: $find_text in group id: {$parent_group->get_id()}", true);

        /** @var Channel $channel */
        $channels = array();
        if ($parent_group->get_id() === ALL_CHANNEL_GROUP_ID) {
            foreach($this->plugin->tv->get_channels() as $channel) {
                if (!is_null($channel) && $channel->is_disabled()) continue;

                foreach ($channel->get_groups() as $group) {
                    if (!$group->is_disabled()) {
                        $channels[] = $channel;
                        break;
                    }
                }
            }
        } else {
            foreach ($parent_group->get_group_channels() as $channel) {
                if (!is_null($channel) && !$channel->is_disabled()) {
                    $channels[] = $channel;
                }
            }
        }

        $defs = array();
        $q_result = false;
        $idx = 0;
        foreach ($channels as $channel) {
            $ch_title = $channel->get_title();
            hd_debug_print("Search in: $ch_title", true);
            $s = mb_stripos($ch_title, $find_text, 0, "UTF-8");
            if ($s !== false) {
                $q_result = true;
                hd_debug_print("found channel: $ch_title, idx: $idx", true);
                $add_params['number'] = $idx;
                Control_Factory::add_close_dialog_and_apply_button_title($defs, $this, $add_params,
                    self::ACTION_JUMP_TO_CHANNEL, '', $ch_title, 900);
            }
            ++$idx;
        }

        if ($q_result === false) {
            Control_Factory::add_multiline_label($defs, '', TR::t('tv_screen_not_found'), 6);
            Control_Factory::add_vgap($defs, 20);
            Control_Factory::add_close_dialog_and_apply_button_title($defs, $this, null,
                self::ACTION_CREATE_SEARCH, '', TR::t('new_search'), 300);
        }

        return Action_Factory::show_dialog(TR::t('search'), $defs, true);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url, true);

        $items = array();

        try {
            if ($this->plugin->tv->load_channels() === -1) {
                throw new Exception("Channels not loaded!");
            }

            $this_group = ($media_url->group_id === ALL_CHANNEL_GROUP_ID)
                ? $this->plugin->tv->get_special_group($media_url->group_id)
                : $this->plugin->tv->get_group($media_url->group_id);

            if (is_null($this_group)) {
                throw new Exception("Group $media_url->group_id not found");
            }
            $channels_order = new Hashed_Array();
            /** @var Channel $channel */
            if ($media_url->group_id === ALL_CHANNEL_GROUP_ID) {
                foreach($this->plugin->tv->get_groups()->get_order() as $group_id) {
                    $group = $this->plugin->tv->get_group($group_id);
                    if (is_null($group)) continue;

                    foreach ($group->get_group_channels()->get_order() as $channel_id) {
                        $channels_order->put($channel_id, $channel_id);
                    }
                }
            } else {
                foreach ($this_group->get_group_channels()->get_order() as $channel_id) {
                    $channels_order->put($channel_id, $channel_id);
                }
            }

            foreach ($channels_order as $item) {
                $channel = $this->plugin->tv->get_channel($item);
                if (is_null($channel) || $channel->is_disabled()) continue;

                $zoom_data = $this->plugin->get_channel_zoom($channel->get_id());
                if ($zoom_data === DuneVideoZoomPresets::not_set) {
                    $detailed_info = TR::t('tv_screen_channel_info__3',
                        $channel->get_title(),
                        $channel->get_archive(),
                        implode(", ", $channel->get_epg_ids())
                    );
                } else {
                    $detailed_info = TR::t('tv_screen_channel_info__4',
                        $channel->get_title(),
                        $channel->get_archive(),
                        implode(", ", $channel->get_epg_ids()),
                        TR::load_string(DuneVideoZoomPresets::$zoom_ops[$zoom_data])
                    );
                }

                $items[] = array(
                    PluginRegularFolderItem::media_url => MediaURL::encode(array('channel_id' => $channel->get_id(), 'group_id' => $this_group->get_id())),
                    PluginRegularFolderItem::caption => $channel->get_title(),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => $channel->get_icon_url(),
                        ViewItemParams::item_detailed_icon_path => $channel->get_icon_url(),
                        ViewItemParams::item_detailed_info => $detailed_info,
                    ),
                    PluginRegularFolderItem::starred => $this->plugin->get_favorites()->in_order($channel->get_id()),
                );
            }
        } catch (Exception $ex) {
            hd_debug_print("Failed collect folder items!");
            print_backtrace_exception($ex);
        }

        return $items;
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('icons_4x3_caption'),
            $this->plugin->get_screen_view('icons_4x3_no_caption'),
            $this->plugin->get_screen_view('icons_3x3_caption'),
            $this->plugin->get_screen_view('icons_3x3_no_caption'),
            $this->plugin->get_screen_view('icons_5x3_caption'),
            $this->plugin->get_screen_view('icons_5x3_no_caption'),
            $this->plugin->get_screen_view('icons_5x4_caption'),
            $this->plugin->get_screen_view('icons_5x4_no_caption'),

            $this->plugin->get_screen_view('icons_7x4_no_caption'),
            $this->plugin->get_screen_view('icons_7x4_caption'),

            $this->plugin->get_screen_view('list_1x11_info'),
            $this->plugin->get_screen_view('list_2x11_small_info'),
            $this->plugin->get_screen_view('list_3x11_no_info'),
        );
    }
}

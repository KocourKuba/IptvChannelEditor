<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channels_setup_screen.php';

class Starnet_Tv_Groups_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_groups';

    const ACTION_CONFIRM_DLG_APPLY = 'apply_dlg';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);

        $action_settings = User_Input_Handler_Registry::create_action($this, ACTION_SETTINGS, TR::t('entry_setup'));

        $actions = array(
            GUI_EVENT_KEY_ENTER      => User_Input_Handler_Registry::create_action($this, ACTION_OPEN_FOLDER),
            GUI_EVENT_KEY_PLAY       => User_Input_Handler_Registry::create_action($this, ACTION_PLAY_FOLDER),
            GUI_EVENT_KEY_B_GREEN    => User_Input_Handler_Registry::create_action($this, ACTION_CHANNELS_SETTINGS, TR::t('tv_screen_channels_setup')),
            GUI_EVENT_KEY_SETUP      => $action_settings,
            GUI_EVENT_KEY_D_BLUE     => $action_settings,
            GUI_EVENT_KEY_POPUP_MENU => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_KEY_RETURN     => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_RETURN),
            GUI_EVENT_KEY_TOP_MENU   => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_TOP_MENU),
            GUI_EVENT_TIMER          => User_Input_Handler_Registry::create_action($this, GUI_EVENT_TIMER),
        );

        if ($this->IsSetupNeeds() !== false) {
            hd_debug_print("Create setup action");
            $configure = User_Input_Handler_Registry::create_action($this, ACTION_NEED_CONFIGURE);
            $actions[GUI_EVENT_KEY_PLAY] = $configure;
            $actions[GUI_EVENT_KEY_ENTER] = $configure;
        }

        if ($this->plugin->config->get_feature(Plugin_Constants::BALANCE_SUPPORTED)) {
            $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this, ACTION_BALANCE, TR::t('tv_screen_subscription'));
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

        $parent_media_url = MediaURL::decode($user_input->parent_media_url);

        switch ($user_input->control_id) {
            case GUI_EVENT_KEY_TOP_MENU:
            case GUI_EVENT_KEY_RETURN:
                if ($this->plugin->get_bool_parameter(PARAM_ASK_EXIT, false)) {
                    return Action_Factory::show_confirmation_dialog(TR::t('yes_no_confirm_msg'), $this, self::ACTION_CONFIRM_DLG_APPLY);
                }

                return User_Input_Handler_Registry::create_action($this, self::ACTION_CONFIRM_DLG_APPLY);

            case GUI_EVENT_TIMER:
                $epg_manager = $this->plugin->get_epg_manager();
                if ($epg_manager === null) {
                    return null;
                }

                clearstatcache();

                $actions = $this->get_action_map($parent_media_url, $plugin_cookies);
                $res = $epg_manager->import_indexing_log();
                if ($res !== false) {
                    foreach (array('pl_last_error', 'xmltv_last_error') as $last_error) {
                        $error_msg = HD::get_last_error($last_error);
                        if (!empty($error_msg)) {
                            return Action_Factory::show_title_dialog(TR::t('err_load_playlist'), null, $error_msg);
                        }
                    }
                    return null;
                }

                return Action_Factory::change_behaviour($actions, 1000);

            case self::ACTION_CONFIRM_DLG_APPLY:
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::epfs_invalidate_folders(null, Action_Factory::close_and_run());

            case ACTION_NEED_CONFIGURE:
                if ($this->IsSetupNeeds()) {
                    hd_debug_print("Setup required!");
                    return Action_Factory::show_title_dialog(TR::t('err_no_account_data'),
                        Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), TR::t('entry_setup')));
                }

                return Action_Factory::open_folder($user_input->selected_media_url);

            case ACTION_OPEN_FOLDER:
            case ACTION_PLAY_FOLDER:
                $post_action = $user_input->control_id === ACTION_OPEN_FOLDER ? Action_Factory::open_folder() : Action_Factory::tv_play();
                $has_error = $this->plugin->config->get_last_error();
                if (!empty($has_error)) {
                    $this->plugin->config->clear_last_error();
                    return Action_Factory::show_title_dialog(TR::t('err_load_any'), $post_action, $has_error);
                }

                return $post_action;

            case ACTION_SETTINGS:
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), TR::t('entry_setup'));

            case ACTION_CHANNELS_SETTINGS:
                return Action_Factory::open_folder(Starnet_Channels_Setup_Screen::get_media_url_str(), TR::t('tv_screen_channels_setup'));

            case ACTION_BALANCE:
                $defs = array();
                $this->plugin->config->AddSubscriptionUI($defs);
                Control_Factory::add_close_dialog_button($defs, TR::t('ok'), 150);
                return Action_Factory::show_dialog(TR::t('tv_screen_information'), $defs);

            case GUI_EVENT_KEY_POPUP_MENU:
                $cache_engine = $this->plugin->get_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_JSON);
                $menu_items = array();
                if ($cache_engine === ENGINE_XMLTV) {
                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_RELOAD,
                        TR::t('refresh_epg'),
                        "refresh.png",
                        array('reload_action' => 'epg'));
                }
                return Action_Factory::show_popup_menu($menu_items);

            case ACTION_CHANGE_EPG_SOURCE:
                hd_debug_print("Start event popup menu for epg source");
                return User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU, null, array(ACTION_CHANGE_EPG_SOURCE => true));

            case ACTION_RELOAD:
                if ($user_input->reload_action === 'epg') {
                    $this->plugin->init_epg_manager();
                    $this->plugin->get_epg_manager()->clear_epg_cache();
                    $this->plugin->run_bg_epg_indexing();

                    $actions = $this->get_action_map($parent_media_url, $plugin_cookies);
                    return Action_Factory::change_behaviour($actions, 1000);
                }

                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Action_Factory::invalidate_all_folders($plugin_cookies);
        }

        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url, true);

        if ($this->plugin->tv->load_channels() === -1) {
            hd_debug_print("Channels not loaded");
        }

        $vod_group = $this->plugin->tv->get_special_group(VOD_GROUP_ID);
        if ($vod_group !== null && !$vod_group->is_disabled()) {
            $vod_item = array(
                PluginRegularFolderItem::media_url => MediaURL::encode(
                    array('screen_id' => Starnet_Vod_Category_List_Screen::ID, 'name' => 'VOD')
                ),
                PluginRegularFolderItem::caption => TR::t(Default_Group::VOD_GROUP_CAPTION),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_LIGHTGREEN,
                    ViewItemParams::icon_path => $vod_group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $vod_group->get_icon_url()
                )
            );
        }

        $items = array();

        /** @var Group $special_group */
        foreach ($this->plugin->tv->get_special_groups() as $special_group) {
            if ($special_group === null || $special_group->get_id() === VOD_GROUP_ID) continue;

            hd_debug_print("group: {$special_group->get_title()} disabled: " . var_export($special_group->is_disabled(), true), true);
            if ($special_group->is_disabled()) continue;

            if ($special_group->get_id() === HISTORY_GROUP_ID) {
                $color = DEF_LABEL_TEXT_COLOR_TURQUOISE;
                $size = $this->plugin->get_playback_points()->size();
            } else {
                if ($special_group->get_id() === ALL_CHANNEL_GROUP_ID) {
                    $color = DEF_LABEL_TEXT_COLOR_SKYBLUE;
                    $size = 0;
                    foreach($this->plugin->tv->get_groups()->get_order() as $group_id) {
                        $group = $this->plugin->tv->get_group($group_id);
                        if (!is_null($group)) {
                            $size += $group->get_group_channels()->size();
                        }
                    }

                } else if ($special_group->get_id() === FAVORITES_GROUP_ID) {
                    $size = $this->plugin->get_favorites()->size();
                    $color = DEF_LABEL_TEXT_COLOR_GOLD;
                } else {
                    $size = $special_group->get_group_channels()->size();
                    $color = DEF_LABEL_TEXT_COLOR_WHITE;
                }
            }

            $item_detailed_info = TR::t('tv_screen_group_info__2', $special_group->get_title(), $size);

            $items[] = array(
                PluginRegularFolderItem::media_url => $special_group->get_media_url_str(),
                PluginRegularFolderItem::caption => $special_group->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::item_caption_color => $color,
                    ViewItemParams::icon_path => $special_group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $special_group->get_icon_url(),
                    ViewItemParams::item_detailed_info => $item_detailed_info,
                )
            );
        }

        $vod_last = $this->plugin->get_parameter(PARAM_VOD_LAST,SwitchOnOff::off) === SwitchOnOff::on;
        if (isset($vod_item) && !$vod_last) {
            $items[] = $vod_item;
        }

        /** @var Default_Group $group */
        foreach ($this->plugin->tv->get_groups() as $group) {
            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Tv_Channel_List_Screen::get_media_url_string($group->get_id()),
                PluginRegularFolderItem::caption => $group->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_WHITE,
                    ViewItemParams::icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_info => TR::t('tv_screen_group_info__2',
                        str_replace('|', '¦', $group->get_title()),
                        $group->get_group_channels()->size()),
                )
            );
        }

        if (isset($vod_item) && $vod_last) {
            $items[] = $vod_item;
        }

        return $items;
    }

    /**
     * @inheritDoc
     */
    public function get_timer(MediaURL $media_url, $plugin_cookies)
    {
        return Action_Factory::timer(1000);
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('icons_4x3_no_caption'),
            $this->plugin->get_screen_view('icons_4x3_caption'),
            $this->plugin->get_screen_view('icons_3x3_no_caption'),
            $this->plugin->get_screen_view('icons_3x3_caption'),
            $this->plugin->get_screen_view('icons_5x3_no_caption'),
            $this->plugin->get_screen_view('icons_5x3_caption'),
            $this->plugin->get_screen_view('icons_5x4_no_caption'),
            $this->plugin->get_screen_view('icons_5x4_caption'),

            $this->plugin->get_screen_view('list_1x11_info'),
            $this->plugin->get_screen_view('list_2x11_small_info'),
            $this->plugin->get_screen_view('list_3x11_no_info'),
        );
    }

    /**
     * @return bool
     */
    protected function IsSetupNeeds()
    {
        switch ($this->plugin->config->get_feature(Plugin_Constants::ACCESS_TYPE)) {
            case Plugin_Constants::ACCOUNT_OTT_KEY:
                $ott_key = $this->plugin->get_credentials(Ext_Params::M_OTT_KEY);
                $subdomain = $this->plugin->get_credentials(Ext_Params::M_SUBDOMAIN);
                $setup_needs = empty($ott_key) && empty($subdomain) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case Plugin_Constants::ACCOUNT_LOGIN:
                $login = $this->plugin->get_credentials(Ext_Params::M_LOGIN);
                $password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
                $setup_needs = empty($login) && empty($password);
                break;
            case Plugin_Constants::ACCOUNT_PIN:
                $password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
                $setup_needs = empty($password);
                break;
            default:
                hd_debug_print("Access type not set");
                return false;
        }

        return $setup_needs && ($this->plugin->config->get_embedded_account() === null);
    }
}

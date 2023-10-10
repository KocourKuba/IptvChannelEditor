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

        switch ($user_input->control_id) {
            case GUI_EVENT_KEY_TOP_MENU:
            case GUI_EVENT_KEY_RETURN:
                if ($this->plugin->get_bool_parameter(PARAM_ASK_EXIT)) {
                    return Action_Factory::show_confirmation_dialog(TR::t('yes_no_confirm_msg'), $this, self::ACTION_CONFIRM_DLG_APPLY);
                }

                return User_Input_Handler_Registry::create_action($this, self::ACTION_CONFIRM_DLG_APPLY);

            case self::ACTION_CONFIRM_DLG_APPLY:
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders(null, Action_Factory::close_and_run());

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
                    $this->plugin->config->set_last_error('');
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
                if ($cache_engine === ENGINE_JSON) break;

                $menu_items = $this->plugin->epg_source_menu($this);
                return Action_Factory::show_popup_menu($menu_items);

            case ACTION_CHANGE_EPG_SOURCE:
                hd_debug_print("Start event popup menu for epg source");
                return User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU, null, array(ACTION_CHANGE_EPG_SOURCE => true));

            case ACTION_EPG_SOURCE_SELECTED:
                if (!isset($user_input->list_idx)) break;

                $this->plugin->set_active_xmltv_source_key($user_input->list_idx);
                $xmltv_source = $this->plugin->get_all_xmltv_sources()->get($user_input->list_idx);
                $this->plugin->set_active_xmltv_source($xmltv_source);
                $this->plugin->tv->reload_channels();

                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Action_Factory::invalidate_all_folders();

            case ACTION_RELOAD:
                if ($user_input->reload_action === 'epg') {
                    $this->plugin->get_epg_manager()->clear_epg_cache();
                    $this->plugin->init_epg_manager();
                    $this->plugin->get_epg_manager()->start_bg_indexing();
                }

                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Action_Factory::invalidate_all_folders();
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
        hd_debug_print($media_url->get_media_url_str(), true);

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
                    ViewItemParams::icon_path => $vod_group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $vod_group->get_icon_url()
                )
            );
        }

        $items = array();

        /** @var Group $group */
        foreach ($this->plugin->tv->get_special_groups() as $group) {
            if ($group === null) continue;

            hd_debug_print("group: {$group->get_title()} disabled: " . var_export($group->is_disabled(), true), true);
            if ($group->is_disabled() || $group->get_id() === VOD_GROUP_ID) continue;

            if ($group->get_id() === HISTORY_GROUP_ID) {
                $item_detailed_info = TR::t('tv_screen_group_info__2', $group->get_title(), $this->plugin->get_playback_points()->size());
            } else {
                $item_detailed_info = TR::t('tv_screen_group_info__2', $group->get_title(), $group->get_group_channels()->size());
            }

            $items[] = array(
                PluginRegularFolderItem::media_url => $group->get_media_url_str(),
                PluginRegularFolderItem::caption => $group->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_info => $item_detailed_info,
                )
            );
        }

        $vod_last = $this->plugin->get_parameter(PARAM_VOD_LAST,'yes') === 'yes';
        if (isset($vod_item) && !$vod_last) {
            $items[] = $vod_item;
        }

        /** @var Default_Group $group */
        foreach ($this->plugin->tv->get_groups() as $group) {
            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Tv_Channel_List_Screen::get_media_url_string($group->get_id()),
                PluginRegularFolderItem::caption => $group->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_info => TR::t('tv_screen_group_info__2', $group->get_title(), $group->get_group_channels()->size()),
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
                $ott_key = $this->plugin->get_parameter(Ext_Params::M_TOKEN);
                $subdomain = $this->plugin->get_parameter(Ext_Params::M_SUBDOMAIN);
                $setup_needs = empty($ott_key) && empty($subdomain) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case Plugin_Constants::ACCOUNT_LOGIN:
                $login = $this->plugin->get_parameter(Ext_Params::M_LOGIN);
                $password = $this->plugin->get_parameter(Ext_Params::M_PASSWORD);
                $setup_needs = empty($login) && empty($password);
                break;
            case Plugin_Constants::ACCOUNT_PIN:
                $password = $this->plugin->get_parameter(Ext_Params::M_PASSWORD);
                $setup_needs = empty($password);
                break;
            default:
                hd_debug_print("Access type not set");
                return false;
        }

        return $setup_needs && ($this->plugin->config->get_embedded_account() === null);
    }
}

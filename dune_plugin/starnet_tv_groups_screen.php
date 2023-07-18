<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channels_setup_screen.php';

class Starnet_Tv_Groups_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_groups';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_TV_GROUP_LIST_FOLDER_VIEWS());

        $plugin->create_screen($this);
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        // if token not set force to open setup screen
        //hd_print(__METHOD__);

        $action_settings = User_Input_Handler_Registry::create_action($this, ACTION_SETTINGS, TR::t('entry_setup'));

        $actions = array(
            GUI_EVENT_KEY_ENTER      => User_Input_Handler_Registry::create_action($this, ACTION_OPEN_FOLDER),
            GUI_EVENT_KEY_PLAY       => User_Input_Handler_Registry::create_action($this, ACTION_PLAY_FOLDER),
            GUI_EVENT_KEY_SETUP      => $action_settings,
            GUI_EVENT_KEY_B_GREEN    => $action_settings,
            GUI_EVENT_KEY_D_BLUE     => User_Input_Handler_Registry::create_action($this, ACTION_CHANNELS_SETTINGS, TR::t('tv_screen_channels_setup')),
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            hd_print(__METHOD__ . ": Create setup action");
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
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        switch ($user_input->control_id) {
            case ACTION_NEED_CONFIGURE:
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    hd_print(__METHOD__ . ": Setup required!");
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
                $this->plugin->config->AddSubscriptionUI($defs, $plugin_cookies);
                Control_Factory::add_close_dialog_button($defs, TR::t('ok'), 150);
                return Action_Factory::show_dialog(TR::t('tv_screen_information'), $defs);

            case GUI_EVENT_KEY_RETURN:
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders(null, Action_Factory::close_and_run());
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
        //hd_print(__METHOD__ . ": get_all_folder_items");
        try {
            $this->plugin->tv->ensure_channels_loaded($plugin_cookies);
        } catch (Exception $e) {
            hd_print(__METHOD__ . ": Channels not loaded");
        }

        $items = array();

        /** @var Default_Group $group */
        foreach ($this->plugin->tv->get_groups() as $group) {

            //hd_print("group: {$group->get_title()} , icon: {$group->get_icon_url()}");
            $icons_param = array(
                ViewItemParams::icon_path => $group->get_icon_url(),
                ViewItemParams::item_detailed_icon_path => $group->get_icon_url()
            );

            if ($group->is_favorite_group()) {
                $fav_item = array(
                    PluginRegularFolderItem::media_url => Starnet_Tv_Favorites_Screen::get_media_url_str(),
                    PluginRegularFolderItem::caption => Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                    PluginRegularFolderItem::view_item_params => $icons_param
                );
            } else if ($group->is_all_channels_group()) {
                $all_item = array(
                    PluginRegularFolderItem::media_url => Starnet_Tv_Channel_List_Screen::get_media_url_str(
                        Default_Dune_Plugin::ALL_CHANNEL_GROUP_ID),
                    PluginRegularFolderItem::caption => Default_Dune_Plugin::ALL_CHANNEL_GROUP_CAPTION,
                    PluginRegularFolderItem::view_item_params => $icons_param
                );
            } else if ($group->is_history_group()) {
                $hist_item = array(
                    PluginRegularFolderItem::media_url => Starnet_TV_History_Screen::get_media_url_str(),
                    PluginRegularFolderItem::caption => Default_Dune_Plugin::PLAYBACK_HISTORY_CAPTION,
                    PluginRegularFolderItem::view_item_params => $icons_param
                );
            } else {
                $items[] = array(
                    PluginRegularFolderItem::media_url => Starnet_Tv_Channel_List_Screen::get_media_url_str($group->get_id()),
                    PluginRegularFolderItem::caption => $group->get_title(),
                    PluginRegularFolderItem::view_item_params => $icons_param
                );
            }
        }

        if ($this->plugin->tv->get_vod_group() !== null) {
            $vod_item = array(
                PluginRegularFolderItem::media_url => MediaURL::encode(
                    array('screen_id' => Starnet_Vod_Category_List_Screen::ID, 'name' => 'VOD')
                ),
                PluginRegularFolderItem::caption => Default_Dune_Plugin::VOD_GROUP_CAPTION,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $this->plugin->tv->get_vod_group()->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $this->plugin->tv->get_vod_group()->get_icon_url()
                )
            );
        }

        $vod_last = !isset($plugin_cookies->vod_last) || $plugin_cookies->vod_last === 'yes';
        if (isset($vod_item) && $vod_last === false) {
            array_unshift($items, $vod_item);
        }

        if (isset($all_item) && (!isset($plugin_cookies->show_all) || $plugin_cookies->show_all === 'yes')) {
            array_unshift($items, $all_item);
        }

        if (isset($hist_item) && (!isset($plugin_cookies->show_history) || $plugin_cookies->show_history === 'yes')) {
            array_unshift($items, $hist_item);
        }

        if (isset($fav_item) && (!isset($plugin_cookies->show_favorites) || $plugin_cookies->show_favorites === 'yes')) {
            array_unshift($items, $fav_item);
        }

        if (isset($vod_item) && $vod_last !== false) {
            $items[] = $vod_item;
        }

        //hd_print("Loaded items " . count($items));
        return $items;
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected function IsSetupNeeds($plugin_cookies)
    {
        switch ($this->plugin->config->get_feature(Plugin_Constants::ACCESS_TYPE)) {
            case Plugin_Constants::ACCOUNT_OTT_KEY:
                $setup_needs = empty($plugin_cookies->ott_key) && empty($plugin_cookies->subdomain) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case Plugin_Constants::ACCOUNT_LOGIN:
                $setup_needs = empty($plugin_cookies->login) && empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case Plugin_Constants::ACCOUNT_PIN:
                $setup_needs = empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            default:
                hd_print(__METHOD__ . ": Access type not set");
                $setup_needs = false;
        }

        return $setup_needs;
    }
}

<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

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
        // hd_print('main_screen: get_action_map');

        $action_settings = User_Input_Handler_Registry::create_action($this, ACTION_SETTINGS, 'Настройки плагина');

        $actions = array(
            GUI_EVENT_KEY_ENTER      => Action_Factory::open_folder(),
            GUI_EVENT_KEY_PLAY       => Action_Factory::tv_play(),
            GUI_EVENT_KEY_SETUP      => $action_settings,
            GUI_EVENT_KEY_B_GREEN    => $action_settings,
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            hd_print("Create setup action");
            $configure = User_Input_Handler_Registry::create_action($this, ACTION_NEED_CONFIGURE);
            $actions[GUI_EVENT_KEY_PLAY] = $configure;
            $actions[GUI_EVENT_KEY_ENTER] = $configure;
        }

        if ($this->plugin->config->get_feature(Plugin_Constants::BALANCE_SUPPORTED)) {
            $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this, ACTION_BALANCE, 'Подписка');
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
        //hd_print('Starnet_Tv_Groups_Screen: handle_user_input:');
        //foreach ($user_input as $key => $value) hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case ACTION_NEED_CONFIGURE:
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    hd_print("Setup required!");
                    return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');
                }

                return Action_Factory::open_folder($user_input->selected_media_url);

            case ACTION_SETTINGS:
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');

            case ACTION_BALANCE:
                $defs = array();
                $this->plugin->config->AddSubscriptionUI($defs, $plugin_cookies);
                Control_Factory::add_close_dialog_button($defs, 'OK', 150);
                return Action_Factory::show_dialog('Подписка', $defs);

            case GUI_EVENT_KEY_RETURN:
                $post_action = Action_Factory::close_and_run();
                if ($this->plugin->history_support) {
                    Playback_Points::init();
                    Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                    $post_action = Starnet_Epfs_Handler::invalidate_folders(null, $post_action);
                }

                return $post_action;
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
        try {
            $this->plugin->tv->ensure_channels_loaded($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Channels not loaded");
        }

        $items = array();

        foreach ($this->plugin->tv->get_groups() as $group) {
            $media_url_str = $group->is_favorite_group() ?
                Starnet_Tv_Favorites_Screen::get_media_url_str() :
                Starnet_Tv_Channel_List_Screen::get_media_url_str($group->get_id());

            $items[] = array(
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $group->get_title(),
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $group->get_icon_url()
                )
            );
        }

        $this->plugin->tv->add_special_groups($items);

        // hd_print("Loaded items " . count($items));
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
                hd_print("Access type not set");
                $setup_needs = false;
        }

        return $setup_needs;
    }
}

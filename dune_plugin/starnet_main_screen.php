<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

class Starnet_Main_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'main_screen';

    const ACTION_SETTINGS = 'settings';
    const ACTION_CONFIGURE = 'configure';
    const ACTION_BALANCE = 'balance';

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
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        // if token not set force to open setup screen
        // hd_print('main_screen: get_action_map');

        $add_settings = User_Input_Handler_Registry::create_action($this, self::ACTION_SETTINGS);
        $add_settings['caption'] = 'Настройки плагина';

        $action = array(
            GUI_EVENT_KEY_SETUP => $add_settings,
            GUI_EVENT_KEY_B_GREEN => $add_settings,
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_PLAY => Action_Factory::tv_play(),
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            hd_print("Create setup action");
            $configure = User_Input_Handler_Registry::create_action($this, self::ACTION_CONFIGURE);
            $action[GUI_EVENT_KEY_PLAY] = $configure;
            $action[GUI_EVENT_KEY_ENTER] = $configure;
        }

        if ($this->plugin->config->get_feature(BALANCE_SUPPORTED)) {
            $add_balance = User_Input_Handler_Registry::create_action($this, self::ACTION_BALANCE);
            $add_balance['caption'] = 'Подписка';
            $action[GUI_EVENT_KEY_C_YELLOW] = $add_balance;
        }

        return $action;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('main_screen_user_input: ' . json_encode($user_input));

        switch ($user_input->control_id) {
            case self::ACTION_CONFIGURE:
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    hd_print("Setup required!");
                    return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');
                }

                return Action_Factory::open_folder($user_input->selected_media_url);

            case self::ACTION_SETTINGS:
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');

            case self::ACTION_BALANCE:
                $defs = array();
                $this->plugin->config->AddSubscriptionUI($defs, $plugin_cookies);
                Control_Factory::add_close_dialog_button($defs, 'OK', 150);
                return Action_Factory::show_dialog('Подписка', $defs);
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
            $media_url_str = $group->is_favorite_channels() ?
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
     * @param MediaURL $media_url
     * @return Archive|null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->tv->get_archive($media_url);
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected function IsSetupNeeds($plugin_cookies)
    {
        switch ($this->plugin->config->get_feature(ACCESS_TYPE)) {
            case ACCOUNT_OTT_KEY:
                $setup_needs = empty($plugin_cookies->ott_key) && empty($plugin_cookies->subdomain) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case ACCOUNT_LOGIN:
                $setup_needs = empty($plugin_cookies->login) && empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case ACCOUNT_PIN:
                $setup_needs = empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            default:
                hd_print("Unknown access type");
                $setup_needs = false;
        }

        return $setup_needs;
    }
}

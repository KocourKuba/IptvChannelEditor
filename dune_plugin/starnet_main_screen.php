<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

class Starnet_Main_Screen extends Tv_Group_List_Screen implements User_Input_Handler
{
    const ID = 'main_screen';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct($plugin);

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

        $add_settings = User_Input_Handler_Registry::create_action($this, 'settings');
        $add_settings['caption'] = 'Настройки плагина';

        $action = array(
            GUI_EVENT_KEY_B_GREEN => $add_settings,
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_PLAY => Action_Factory::tv_play(),
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            hd_print("Create setup action");
            $action[GUI_EVENT_KEY_PLAY] = User_Input_Handler_Registry::create_action($this, 'configure');
            $action[GUI_EVENT_KEY_ENTER] = User_Input_Handler_Registry::create_action($this, 'configure');
        }

        if ($this->plugin->config->get_feature(BALANCE_SUPPORTED)) {
            $add_balance = User_Input_Handler_Registry::create_action($this, 'check_balance');
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
            case 'configure':
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    hd_print("Setup required!");
                    return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');
                }

                return Action_Factory::open_folder($user_input->selected_media_url);

            case 'settings':
                return Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');

            case 'check_balance':
                $defs = array();
                $this->plugin->config->AddSubscriptionUI($defs, $plugin_cookies);
                Control_Factory::add_close_dialog_button($defs, 'OK', 150);
                return Action_Factory::show_dialog('Подписка', $defs);
        }

        return null;
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected function IsSetupNeeds($plugin_cookies)
    {
        switch ($this->plugin->config->get_feature(ACCOUNT_TYPE)) {
            case 'OTT_KEY':
                $setup_needs = empty($plugin_cookies->ott_key) && empty($plugin_cookies->subdomain) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case 'LOGIN':
                $setup_needs = empty($plugin_cookies->login) && empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            case 'PIN':
                $setup_needs = empty($plugin_cookies->password) && ($this->plugin->config->get_embedded_account() === null);
                break;
            default:
                hd_print("Unknown plugin type");
                $setup_needs = false;
        }

        return $setup_needs;
    }
}

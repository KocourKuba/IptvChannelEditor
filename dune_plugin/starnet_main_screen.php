<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

class StarnetMainScreen extends TvGroupListScreen implements UserInputHandler
{
    const ID = 'main_screen';

    ///////////////////////////////////////////////////////////////////////

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct($plugin);

        $plugin->create_screen($this);
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        // if token not set force to open setup screen
        // hd_print('main_screen: get_action_map');

        $add_settings = UserInputHandlerRegistry::create_action($this, 'settings');
        $add_settings['caption'] = 'Настройки плагина';

        $action = array(
            GUI_EVENT_KEY_B_GREEN => $add_settings,
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            hd_print("Create setup action");
            $action[GUI_EVENT_KEY_PLAY] = UserInputHandlerRegistry::create_action($this, 'configure');
            $action[GUI_EVENT_KEY_ENTER] = UserInputHandlerRegistry::create_action($this, 'configure');
        }

        if ($this->plugin->config->get_feature(BALANCE_SUPPORTED)) {
            $add_balance = UserInputHandlerRegistry::create_action($this, 'check_balance');
            $add_balance['caption'] = 'Подписка';
            $action[GUI_EVENT_KEY_C_YELLOW] = $add_balance;
        }

        return $action;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('main_screen_user_input: ' . json_encode($user_input));

        switch ($user_input->control_id) {
            case 'configure':
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    hd_print("Setup required!");
                    return ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина');
                }

                return ActionFactory::open_folder($user_input->selected_media_url);

            case 'settings':
                return ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина');

            case 'check_balance':
                $defs = array();
                $this->plugin->config->AddSubscriptionUI($defs, $plugin_cookies);
                ControlFactory::add_close_dialog_button($defs, 'OK', 150);
                return ActionFactory::show_dialog('Подписка', $defs) ;
        }

        return null;
    }

    protected function IsSetupNeeds($plugin_cookies)
    {
        switch ($this->plugin->config->get_feature(ACCOUNT_TYPE))
        {
            case 'OTT_KEY':
                $setup_needs = (empty($plugin_cookies->ott_key) && empty($plugin_cookies->subdomain) &&
                    (empty($plugin_cookies->ott_key_local) && empty($plugin_cookies->subdomain_local)));
                break;
            case 'LOGIN':
                $setup_needs = (empty($plugin_cookies->login) && empty($plugin_cookies->password)) &&
                    (empty($plugin_cookies->login_local) && empty($plugin_cookies->password_local));
                break;
            case 'PIN':
                $setup_needs = empty($plugin_cookies->password) && empty($plugin_cookies->password_local);
                break;
            default:
                hd_print("Unknown plugin type");
                $setup_needs = false;
        }

        return $setup_needs;
    }
}

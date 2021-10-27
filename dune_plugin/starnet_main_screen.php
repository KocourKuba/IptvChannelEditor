<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

class StarnetMainScreen extends TvGroupListScreen implements UserInputHandler
{
    const ID = 'main_screen';
    public static $config;

    ///////////////////////////////////////////////////////////////////////

    public function __construct(Tv $tv, $folder_views)
    {
        parent::__construct($tv, $folder_views);

        UserInputHandlerRegistry::get_instance()->register_handler($this);
    }

    public function get_handler_id()
    {
        return self::ID;
    }

    private function account_info_dialog(&$plugin_cookies)
    {
        $defs = array();
        try {
            $account_data = array();
            $result = self::$config->GetAccountInfo($plugin_cookies, $account_data, true);
            if ($result === false || empty($account_data)) {
                throw new Exception('Account error');
            }

            $title = 'Пакеты: ';
            $need_collect = false;
            $list = array();
            switch (PLUGIN_TYPE)
            {
                case 'SharaclubPluginConfig':
                    ControlFactory::add_label($defs, 'Баланс:', $account_data['data']['money'] . ' руб.');
                    ControlFactory::add_label($defs, 'Цена подписки:', $account_data['data']['money_need'] . ' руб.');
                    $packages = $account_data['data']['abon'];
                    $str_len = strlen($packages);
                    if ($str_len === 0) {
                        ControlFactory::add_label($defs, $title, 'Нет пакетов');
                    }

                    if($str_len < 30) {
                        ControlFactory::add_label($defs, $title, $packages);
                    }

                    if($str_len >= 30) {
                        $need_collect = true;
                        $list = explode(', ', $packages);
                    }
                    break;
                case 'ItvPluginConfig':
                    ControlFactory::add_label($defs, 'Баланс:', $account_data['user_info']['cash'] . ' $');
                    $packages = $account_data['package_info'];
                    if (count($packages) === 0) {
                        ControlFactory::add_label($defs, $title, 'Нет пакетов');
                    }
                    else {
                        $need_collect = true;
                        foreach ($packages as $item) {
                            $list[] = $item['name'];
                        }
                    }
                    break;
                default:
                    return ActionFactory::show_dialog('Подписка', $defs) ;
            }

            if ($need_collect) {
                $emptyTitle = str_repeat(' ', strlen($title));
                $list_collected = array();
                $isFirstLabel = true;
                foreach($list as $item) {
                    $list_collected[] = $item;
                    $collected = implode(', ', $list_collected);
                    if (strlen($collected) < 30) {
                        continue;
                    }

                    ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected);

                    if ($isFirstLabel) {
                        $isFirstLabel = false;
                    }

                    $list_collected = array();
                }

                if (count($list_collected) !== 0) {
                    ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
                }
            }
        } catch (Exception $ex) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            ControlFactory::add_label($defs, 'Ошибка!', $text[0]);
            ControlFactory::add_label($defs, 'Описание:', $text[1]);
        }

        ControlFactory::add_close_dialog_button($defs, 'OK', 150);

        return ActionFactory::show_dialog('Подписка', $defs) ;
    }

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        // if token not set force to open setup screen
        $config = self::$config;
        switch ($config::$ACCOUNT_TYPE)
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
                return array();
        }

        $add_action = UserInputHandlerRegistry::create_action($this, 'settings');
        $add_action['caption'] = 'Настройки плагина';

        if ($setup_needs !== false) {
            hd_print("Plugin not configured");
            return array(
                GUI_EVENT_KEY_ENTER => $add_action,
                GUI_EVENT_KEY_PLAY => $add_action,
                GUI_EVENT_KEY_B_GREEN => $add_action
            );
        }

        if (PLUGIN_TYPE === 'SharaclubPluginConfig' || PLUGIN_TYPE === 'ItvPluginConfig') {
            $balance = UserInputHandlerRegistry::create_action($this, 'check_balance');
            $balance['caption'] = 'Подписка';
            return array
            (
                GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
                GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
                GUI_EVENT_KEY_B_GREEN => $add_action,
                GUI_EVENT_KEY_C_YELLOW => $balance
            );
        }

        return array
        (
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => $add_action,
        );
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('main_screen: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case 'settings':
                return ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина');
            case 'check_balance':
                return $this->account_info_dialog(&$plugin_cookies);
        }

        return null;
    }
}

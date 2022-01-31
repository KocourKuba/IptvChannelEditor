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
                    } else {
                        $need_collect = true;
                        foreach ($packages as $item) {
                            $list[] = $item['name'];
                        }
                    }
                    break;
                case 'CbillingPluginConfig':
                    ControlFactory::add_label($defs, $title, empty($account_data['data']['package']) ? 'Нет пакетов' : $account_data['data']['package']);
                    ControlFactory::add_label($defs, 'Дата окончания', $account_data['data']['end_date']);
                    ControlFactory::add_label($defs, 'Устройств', $account_data['data']['devices_num']);
                    ControlFactory::add_label($defs, 'Сервер', $account_data['data']['server']);
                    break;
                case 'VidokPluginConfig':
                    ControlFactory::add_label($defs, 'Баланс:', $account_data['account']['balance'] . ' €');
                    ControlFactory::add_label($defs, 'Логин:', $account_data['account']['login']);
                    $packages = $account_data['account']['packages'];
                    if (count($packages) === 0) {
                        ControlFactory::add_label($defs, $title, 'Нет пакетов');
                    } else {
                        foreach ($packages as $item) {
                            ControlFactory::add_label($defs, 'Пакет:', $item['name'] .' до '. date('j.m.Y', $item['expire']));
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
        // hd_print('main_screen: get_action_map');

        $add_settings = UserInputHandlerRegistry::create_action($this, 'settings');
        $add_settings['caption'] = 'Настройки плагина';

        $action = array(
            GUI_EVENT_KEY_B_GREEN => $add_settings,
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
        );

        if ($this->IsSetupNeeds($plugin_cookies) !== false) {
            $action[GUI_EVENT_KEY_ENTER] = UserInputHandlerRegistry::create_action($this, 'configure');
        }

        if (self::$config->get_balance_support()) {
            $add_balance = UserInputHandlerRegistry::create_action($this, 'check_balance');
            $add_balance['caption'] = 'Подписка';
            $action[GUI_EVENT_KEY_C_YELLOW] = $add_balance;
        }

        return $action;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('main_screen: handle_user_input');
        //foreach ($user_input as $key => $value) {
        //    hd_print("  $key => $value");
        //}

        switch ($user_input->control_id) {
            case 'configure':
                if ($this->IsSetupNeeds($plugin_cookies)) {
                    return ActionFactory::show_error(false, 'Плагин не настроен', 'Зайдите в настройки плагина');
                }

                return ActionFactory::open_folder($user_input->selected_media_url);
            case 'settings':
                return ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина');
            case 'check_balance':
                return $this->account_info_dialog(&$plugin_cookies);
        }

        return null;
    }

    protected function IsSetupNeeds($plugin_cookies)
    {
        switch (self::$config->get_account_type())
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

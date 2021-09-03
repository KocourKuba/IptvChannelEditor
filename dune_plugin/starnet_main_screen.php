<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'starnet_setup_screen.php';

class StarnetMainScreen extends TvGroupListScreen implements UserInputHandler
{
    const ID = 'main_screen';

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
            $result = $this->tv->config->GetAccountStatus($plugin_cookies);
            if ($result === false || $result->status != 'ok')
                throw new Exception('Account error');

            ControlFactory::add_label($defs, 'Баланс:', $result->data->money . ' руб.');
            ControlFactory::add_label($defs, 'Цена подписки:', $result->data->money_need . ' руб.');

            $title = 'Пакеты: ';
            $packages = $result->data->abon;
            $str_len = strlen($packages);
            if ($str_len == 0)
                ControlFactory::add_label($defs, $title, 'Нет пакетов');

            if($str_len < 25)
                ControlFactory::add_label($defs, $title, $packages);

            if($str_len >= 25) {
                $isFirstLabel = true;
                $list = explode(', ', $packages);
                $emptyTitle = str_repeat(' ', strlen($title));
                $list_collected = array();
                for($i = 0; $i < count($list); $i++) {
                    array_push($list_collected, $list[$i]);
                    $collected = implode(', ', $list_collected);
                    if (strlen($collected) < 25) continue;

                    ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected);

                    if ($isFirstLabel)
                        $isFirstLabel = false;

                    $list_collected = array();
                }

                if (count($list_collected) != 0)
                    ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
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
        $add_action = UserInputHandlerRegistry::create_action($this, 'settings');
        $add_action['caption'] = 'Настройки плагина';

        // if token not set force to open setup screen
        $setup_needs = isset($plugin_cookies->subdomain) || isset($plugin_cookies->subdomain_local) ? true : false;
        if ($setup_needs === false) {
            $setup_screen = ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки');
            return array(
                GUI_EVENT_KEY_ENTER => $setup_screen,
                GUI_EVENT_KEY_PLAY => $setup_screen,
                GUI_EVENT_KEY_B_GREEN => $setup_screen
            );
        }

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

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('main_screen: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case 'settings':
                return ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки');
            case 'check_balance':
                return $this->account_info_dialog(&$plugin_cookies);
        }

        return null;
    }
}

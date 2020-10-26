<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/abstract_preloaded_regular_screen.php';

require_once 'starnet_setup_screen.php';

///////////////////////////////////////////////////////////////////////////


class Starnet_MainScreen extends TvGroupListScreen implements UserInputHandler
{
    const ID = 'main_screen';

    const Version = '2.0.5';
    ///////////////////////////////////////////////////////////////////////

    protected $tv;

    ///////////////////////////////////////////////////////////////////////

    public function __construct($tv, $folder_views)
    {
        parent::__construct($tv, $folder_views);

        $this->tv = $tv;

        UserInputHandlerRegistry::get_instance()->register_handler($this);
    }

    public function get_handler_id()
    {
        return self::ID;
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $setup_screen = $this->tv->get_setup_screen();
        if (!$setup_screen) {
            $add_action = UserInputHandlerRegistry::create_action($this, 'settings');
        }


        $add_action['caption'] = 'Настройки плагина';

        return array
        (
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => $add_action,
        );
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('main_screen: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case 'settings':
                return ActionFactory::open_folder(DemoSetupScreen::get_media_url_str(), "Настройки");
        }

        return null;
    }
}

///////////////////////////////////////////////////////////////////////////
?>

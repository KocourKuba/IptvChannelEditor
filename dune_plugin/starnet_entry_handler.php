<?php
/** @noinspection PhpUndefinedClassInspection */
require_once 'lib/user_input_handler_registry.php';

class StarnetEntryHandler implements UserInputHandler
{
    const ID = 'entry';

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }
	
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('StarnetEntryHandler: ' . json_encode($user_input));

        if (isset($user_input->control_id)) {
            switch ($user_input->control_id) {
                case 'do_reboot':
                    return ActionFactory::restart(true);
                case 'power_off':
                    return send_ir_code(GUI_EVENT_DISCRETE_POWER_OFF);
                case 'do_setup':
                    return ActionFactory::open_folder('setup', 'Настройки ' . DuneSystem::$properties['plugin_name']);
                case 'launch':
                    if ((int)$user_input->mandratory_playback === 1) {
                        hd_print("launch play");
                        return ActionFactory::tv_play();
                    }

                    hd_print("launch open");
                    return ActionFactory::open_folder();
            }
        }

        return null;
    }
}

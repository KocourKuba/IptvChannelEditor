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
        hd_print('StarnetEntryHandler: ' . json_encode($user_input));

        if (isset($user_input->control_id)) {
            switch ($user_input->control_id) {
                case 'do_reboot':
                    hd_print("do reboot");
                    return ActionFactory::restart(true);
                case 'power_off':
                    hd_print("do power off");
                    return send_ir_code(GUI_EVENT_DISCRETE_POWER_OFF);
                case 'do_setup':
                    hd_print("do setup");
                    return ActionFactory::open_folder('setup', 'Настройки ' . DuneSystem::$properties['plugin_name']);
                case 'plugin_entry':
                    if (isset($user_input->action_id) && $user_input->action_id === 'launch') {
                        if ((int)$user_input->mandatory_playback === 1) {
                            hd_print("action: launch play");
                            return ActionFactory::tv_play();
                        }

                        hd_print("action: launch open");
                        return ActionFactory::open_folder();
                    }
            }
        }

        return null;
    }
}

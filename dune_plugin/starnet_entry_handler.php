<?php
/** @noinspection PhpUndefinedClassInspection */
require_once 'lib/user_input_handler_registry.php';

class StarnetEntryHandler implements UserInputHandler
{
    const ID = 'entry';

    public function get_handler_id()
    {
        return self::ID;
    }
	
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        if (isset($user_input->control_id)) {
            if ($user_input->control_id === 'do_reboot') {
                return shell_exec('reboot');
            }

            if ($user_input->control_id === 'power_off') {
                return shell_exec('echo A15EBF00 > /proc/ir/button');
            }

            if ($user_input->control_id === 'do_setup') {
                return ActionFactory::open_folder('setup', 'Настройки ' . DuneSystem::$properties['plugin_name']);
            }
        }
        if ($user_input->handler_id === 'entry') {
            return ActionFactory::open_folder();
        }

        return null;
    }
}

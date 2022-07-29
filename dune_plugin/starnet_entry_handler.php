<?php

require_once 'lib/user_input_handler_registry.php';

class Starnet_Entry_Handler implements User_Input_Handler
{
    const ID = 'entry';

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('StarnetEntryHandler: ' . json_encode($user_input));

        if (isset($user_input->control_id)) {
            switch ($user_input->control_id) {
                case 'do_reboot':
                    hd_print("do reboot");
                    return Action_Factory::restart(true);
                case 'power_off':
                    hd_print("do power off");
                    return array(send_ir_code(GUI_EVENT_DISCRETE_POWER_OFF));
                case 'do_setup':
                    hd_print("do setup");
                    return Action_Factory::open_folder('setup', 'Настройки ' . DuneSystem::$properties['plugin_name']);
                case 'do_clear_epg':
                    $epg_path = get_temp_path("epg/");
                    hd_print("do clear epg: $epg_path");
                    foreach(glob($epg_path . "*") as $file) {
                        if(is_file($file)) {
                            unlink($file);
                        }
                    }
                    return Action_Factory::show_title_dialog('Кэш EPG очищен');
                case 'plugin_entry':
                    if (isset($user_input->action_id) && $user_input->action_id === 'launch') {
                        if ((int)$user_input->mandatory_playback === 1) {
                            hd_print("action: launch play");
                            return Action_Factory::tv_play();
                        }

                        hd_print("action: launch open");
                        return Action_Factory::open_folder();
                    }
            }
        }

        return null;
    }
}

<?php

require_once 'lib/user_input_handler_registry.php';

class Starnet_Entry_Handler implements User_Input_Handler
{
    const ID = 'entry';

    private $plugin;

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        $this->plugin = $plugin;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('StarnetEntryHandler: handle_user_input');
        //foreach ($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->control_id))
            return null;

        switch ($user_input->control_id) {
            case 'do_reboot':
                hd_print("do reboot");
                if (is_apk()) {
                    return Action_Factory::show_title_dialog('Для этой приставки эта функция недоступна!');
                }
                return Action_Factory::restart(true);

            case 'power_off':
                hd_print("do power off");
                if (is_apk()) {
                    return Action_Factory::show_title_dialog('Для этой приставки эта функция недоступна!');
                }
                return array(send_ir_code(GUI_EVENT_DISCRETE_POWER_OFF));

            case 'do_setup':
                hd_print("do setup");
                return Action_Factory::open_folder('setup', 'Настройки ' . get_plugin_name());

            case 'do_send_log':
                hd_print("do_send_log");
                $error_msg = '';
                $msg = HD::send_log_to_developer($plugin_cookies, $error_msg) ? "Лог отправлен!" : "Лог не отправлен! $error_msg";
                return Action_Factory::show_title_dialog($msg);

            case 'do_clear_epg':
                $this->plugin->tv->clear_epg_cache();
                return Action_Factory::clear_rows_info_cache(Action_Factory::show_title_dialog('Кэш EPG очищен'));

            case 'plugin_entry':
                if (!isset($user_input->action_id)) break;

                hd_print("plugin_entry $user_input->action_id");
                clearstatcache();
                switch ($user_input->action_id) {
                    case 'launch':
                        if (HD::check_https_proxy() !== false) {
                            return Action_Factory::show_title_dialog("Требуется перезагрузка",
                                Action_Factory::restart(), "Плагин настроен на обновление через https прокси");
                        }

                        //hd_print("auto_play: $plugin_cookies->auto_play");
                        if ((int)$user_input->mandatory_playback === 1
                            || (isset($plugin_cookies->auto_play) && $plugin_cookies->auto_play === SetupControlSwitchDefs::switch_on)) {
                            hd_print("action: launch play");

                            $media_url = null;
                            if (file_exists('/config/resume_state.properties')) {
                                $resume_state = parse_ini_file('/config/resume_state.properties', 0, INI_SCANNER_RAW);

                                if (strpos($resume_state['plugin_name'], get_plugin_name()) !== false) {
                                    $media_url = MediaURL::decode();
                                    $media_url->is_favorite = $resume_state['plugin_tv_is_favorite'];
                                    $media_url->group_id = $resume_state['plugin_tv_is_favorite'] ? Starnet_Tv_Favorites_Screen::ID : $resume_state['plugin_tv_group'];
                                    $media_url->channel_id = $resume_state['plugin_tv_channel'];
                                    $media_url->archive_tm = ((time() - $resume_state['plugin_tv_archive_tm']) < 259200) ? $resume_state['plugin_tv_archive_tm'] : -1;
                                }
                            }
                            $action = Action_Factory::tv_play($media_url);
                        } else {
                            hd_print("action: launch open");
                            $action = Action_Factory::open_folder();
                        }

                        return $action;

                    case 'auto_resume':
                        //hd_print("auto_resume: $plugin_cookies->auto_resume");
                        if ((int)$user_input->mandatory_playback !== 1
                            || (isset($plugin_cookies->auto_resume) && $plugin_cookies->auto_resume === SetupControlSwitchDefs::switch_off)) break;

                        $media_url = null;
                        if (file_exists('/config/resume_state.properties')) {
                            $resume_state = parse_ini_file('/config/resume_state.properties', 0, INI_SCANNER_RAW);

                            if (strpos($resume_state['plugin_name'], get_plugin_name()) !== false) {
                                $media_url = MediaURL::decode();
                                $media_url->is_favorite = $resume_state['plugin_tv_is_favorite'];
                                $media_url->group_id = $resume_state['plugin_tv_is_favorite'] ? Starnet_Tv_Favorites_Screen::ID : $resume_state['plugin_tv_group'];
                                $media_url->channel_id = $resume_state['plugin_tv_channel'];
                                $media_url->archive_tm = ((time() - $resume_state['plugin_tv_archive_tm']) < 259200) ? $resume_state['plugin_tv_archive_tm'] : -1;
                            }
                        }

                        return Action_Factory::tv_play($media_url);

                    case 'update_epfs':
                        hd_print("action: update_epfs");
                        return Starnet_Epfs_Handler::update_all_epfs($plugin_cookies, isset($user_input->first_run_after_boot) || isset($user_input->restore_from_sleep));
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        return null;
    }
}

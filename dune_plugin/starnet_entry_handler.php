<?php

require_once 'lib/user_input_handler_registry.php';

class Starnet_Entry_Handler implements User_Input_Handler
{
    const ID = 'entry';
    const ACTION_PLUGIN_ENTRY = 'plugin_entry';
    const ACTION_LAUNCH = 'launch';
    const ACTION_LAUNCH_VOD = 'launch_vod';
    const ACTION_AUTO_RESUME = 'auto_resume';
    const ACTION_UPDATE_EPFS = 'update_epfs';
    const ACTION_INSTALL = 'install';
    const ACTION_UNINSTALL = 'uninstall';
    const ACTION_UPDATE = 'update';
    const ACTION_DO_CHANNELS_SETTINGS = 'do_channels_setup';
    const ACTION_DO_PLUGIN_SETTINGS = 'do_setup';
    const ACTION_DO_REBOOT = 'do_reboot';
    const ACTION_DO_POWER_OFF = 'power_off';
    const ACTION_DO_SEND_LOG = 'do_send_log';
    const ACTION_DO_CLEAR_EPG = 'do_clear_epg';

    private $plugin;

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        $this->plugin = $plugin;
    }

    /**
     * @inheritDoc
     */
    public static function get_handler_id()
    {
        return static::ID . '_handler';
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        if (!isset($user_input->control_id))
            return null;

        hd_debug_print("user input control: $user_input->control_id", true);

        switch ($user_input->control_id) {
            case self::ACTION_DO_REBOOT:
                return Action_Factory::restart(true);

            case self::ACTION_DO_POWER_OFF:
                if (is_limited_apk()) {
                    return Action_Factory::show_title_dialog(TR::t('entry_not_available'));
                }
                return array(send_ir_code(GUI_EVENT_DISCRETE_POWER_OFF));

            case self::ACTION_DO_PLUGIN_SETTINGS:
                $this->plugin->tv->reload_channels();
                return Action_Factory::open_folder('setup', TR::t('entry_setup'));

            case self::ACTION_DO_CHANNELS_SETTINGS:
                $this->plugin->tv->reload_channels();
                return Action_Factory::open_folder('channels_setup', TR::t('tv_screen_channels_setup'));

            case self::ACTION_DO_SEND_LOG:
                if (is_newer_versions()) {
                    $error_msg = '';
                    $msg = HD::send_log_to_developer($error_msg) ? TR::t('entry_log_sent') : TR::t('entry_log_not_sent') . " $error_msg";
                    return Action_Factory::show_title_dialog($msg);
                }

                return Action_Factory::show_title_dialog(TR::t('entry_log_not_sent_too_old'));

            case self::ACTION_DO_CLEAR_EPG:
                $this->plugin->clear_all_epg_cache();
                $this->plugin->tv->unload_channels();
                $post_action = Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));
                return HD::rows_api_support() ? Action_Factory::clear_rows_info_cache($post_action) : $post_action;

            case self::ACTION_PLUGIN_ENTRY:
                if (!isset($user_input->action_id)) break;

                hd_debug_print("plugin_entry $user_input->action_id");

                if (!is_newer_versions()) {
                    hd_debug_print("Too old Dune HD firmware! " . get_raw_firmware_version());
                    return  Action_Factory::show_error(true, TR::t('err_too_old_player'),
                        array(
                            TR::load_string('err_too_old_player'),
                            "Dune Product ID: " .get_product_id(),
                            "Dune Firmware: " . get_raw_firmware_version(),
                            "Dune Serial: " . get_serial_number(),
                        ));
                }

                switch ($user_input->action_id) {
                    case self::ACTION_LAUNCH:
                        hd_debug_print_separator();
                        hd_debug_print("LANUCH PLUGIN");
                        hd_debug_print_separator();

                        if (toggle_updater_proxy($this->plugin->get_bool_parameter(PARAM_USE_UPDATER_PROXY, false))) {
                            return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'),
                                Action_Factory::restart(), TR::t('entry_updater_proxy_enabled'));
                        }

                        $this->plugin->tv->reload_channels();

                        if ((int)$user_input->mandatory_playback === 1
                            || (isset($plugin_cookies->auto_play) && $plugin_cookies->auto_play === SetupControlSwitchDefs::switch_on)) {
                            hd_debug_print("launch play");

                            $action = Action_Factory::tv_play($this->get_resume_mediaurl());
                        } else {
                            hd_debug_print("action: launch open");
                            $action = Action_Factory::open_folder();
                        }

                        return $action;

                    case self::ACTION_AUTO_RESUME:
                        hd_debug_print_separator();
                        hd_debug_print("LANUCH PLUGIN AUTO RESUME MODE");
                        hd_debug_print_separator();

                        $this->plugin->tv->reload_channels();

                        if ((int)$user_input->mandatory_playback !== 1
                            || (isset($plugin_cookies->auto_resume) && $plugin_cookies->auto_resume === SetupControlSwitchDefs::switch_off)) break;

                        return Action_Factory::tv_play($this->get_resume_mediaurl());

                    case self::ACTION_UPDATE_EPFS:
                        $this->plugin->init_epg_manager();
                        return Starnet_Epfs_Handler::update_all_epfs($plugin_cookies,
                            isset($user_input->first_run_after_boot) || isset($user_input->restore_from_sleep));

                    case self::ACTION_INSTALL:
                    case self::ACTION_UPDATE:
                        $this->plugin->upgrade_old_settings($plugin_cookies);
                        toggle_updater_proxy($this->plugin->get_bool_parameter(PARAM_USE_UPDATER_PROXY, false));
                        break;

                    case self::ACTION_UNINSTALL:
                        $this->plugin->clear_all_epg_cache();
                        break;

                    default:
                        break;
                }
                break;
            default:
                break;
        }

        return null;
    }

    private function get_resume_mediaurl()
    {
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
        return $media_url;
    }
}

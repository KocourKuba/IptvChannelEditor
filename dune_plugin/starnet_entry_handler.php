<?php

require_once 'lib/user_input_handler_registry.php';
require_once 'screens_tv/starnet_tv_favorites_screen.php';

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
    const ACTION_FORCE_OPEN = 'force_open';
    const ACTION_EXTERNAL_SEARCH = 'external_search';

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
    public function get_handler_id()
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
                if (is_r11_or_higher()) {
                    $error_msg = '';
                    $msg = HD::send_log_to_developer($error_msg) ? TR::t('entry_log_sent') : TR::load('entry_log_not_sent') . " $error_msg";
                    return Action_Factory::show_title_dialog($msg);
                }

                return Action_Factory::show_title_dialog(TR::t('entry_log_not_sent_too_old'));

            case self::ACTION_DO_CLEAR_EPG:
                $this->plugin->clear_all_epg_cache();
                $this->plugin->tv->unload_channels();
                $post_action = Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));
                return HD::rows_api_support() ? Action_Factory::clear_rows_info_cache($post_action) : $post_action;

            case self::ACTION_FORCE_OPEN:
                hd_debug_print_separator();
                hd_debug_print("FORCE LANUCH PLUGIN");
                hd_debug_print_separator();

                $value = $this->plugin->get_bool_setting(PARAM_USE_UPDATER_PROXY, false);
                if (toggle_updater_proxy($value)) {
                    $this->plugin->set_bool_setting(PARAM_USE_UPDATER_PROXY, true);
                    return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'),
                        TR::t('entry_updater_proxy_enabled'), Action_Factory::restart());
                }

                $this->plugin->tv->reload_channels();

                return Action_Factory::refresh_entry_points(
                    Action_Factory::invalidate_all_folders($plugin_cookies,
                        Action_Factory::open_folder(Starnet_Tv_Groups_Screen::ID)
                    )
                );

            case self::ACTION_EXTERNAL_SEARCH:
                return $this->do_external_search($user_input);

            case self::ACTION_PLUGIN_ENTRY:
                if (!isset($user_input->action_id)) break;

                hd_debug_print("plugin_entry $user_input->action_id");

                if (!is_r11_or_higher()) {
                    hd_debug_print("Too old Dune HD firmware! " . get_raw_firmware_version());
                    return  Action_Factory::show_error(true, TR::t('err_too_old_player'),
                        array(
                            TR::load('err_too_old_player'),
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

                        $value = $this->plugin->get_bool_setting(PARAM_USE_UPDATER_PROXY, false);
                        if (toggle_updater_proxy($value)) {
                            $this->plugin->set_bool_setting(PARAM_USE_UPDATER_PROXY, true);
                            return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'),
                                TR::t('entry_updater_proxy_enabled'), Action_Factory::restart());
                        }

                        $this->plugin->tv->reload_channels();

                        if ((int)$user_input->mandatory_playback === 1
                            || (isset($plugin_cookies->auto_play) && SwitchOnOff::to_bool($plugin_cookies->auto_play))) {
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
                            || (isset($plugin_cookies->auto_resume) && !SwitchOnOff::to_bool($plugin_cookies->auto_resume))) break;

                        return Action_Factory::tv_play($this->get_resume_mediaurl());

                    case self::ACTION_UPDATE_EPFS:
                        $this->plugin->init_epg_manager();
                        return Starnet_Epfs_Handler::update_all_epfs($plugin_cookies,
                            isset($user_input->first_run_after_boot) || isset($user_input->restore_from_sleep));

                    case self::ACTION_INSTALL:
                    case self::ACTION_UPDATE:
                        $this->plugin->upgrade_old_settings($plugin_cookies);
                        toggle_updater_proxy($this->plugin->get_bool_setting(PARAM_USE_UPDATER_PROXY, false));
                        return Action_Factory::refresh_entry_points();

                    case self::ACTION_UNINSTALL:
                        $this->plugin->clear_all_epg_cache();
                        return Action_Factory::refresh_entry_points();

                    default:
                        break;
                }
                break;
            default:
                break;
        }

        return null;
    }

    /**
     * Search in VOD called by other plugins (Play in apps) through public_actions/vod_search.
     * Param: search - plain string or json encoded array with title, original_title (results of both are merged)
     *
     * @param object $user_input
     * @return array
     */
    private function do_external_search($user_input)
    {
        hd_debug_print_separator();
        hd_debug_print("EXTERNAL SEARCH");
        hd_debug_print_separator();

        if (!is_r11_or_higher()) {
            hd_debug_print("Too old Dune HD firmware! " . get_raw_firmware_version());
            return Action_Factory::show_title_dialog(TR::t('err_too_old_player'));
        }

        // checked before the account: data required only for VOD (edem vportal) are not account data
        if (!$this->plugin->config->is_vod_enabled()) {
            hd_debug_print("VOD not supported or disabled");
            return Action_Factory::show_title_dialog(TR::t('err_vod_not_available'));
        }

        if ($this->plugin->config->is_vod_account_required() && !$this->plugin->is_account_set()) {
            hd_debug_print("Account data not set");
            return Action_Factory::show_title_dialog(TR::t('err_no_account_data'));
        }

        $search_string = isset($user_input->search) ? trim($user_input->search) : '';
        $keywords = $search_string === '' ? array() : default_config::decode_search_keywords($search_string);
        if (empty($keywords)) {
            hd_debug_print("Nothing to search");
            return Action_Factory::show_title_dialog(TR::t('err_search_empty'));
        }

        hd_debug_print("search: $search_string");

        if ($this->plugin->config->is_vod_account_required()) {
            // account info, provider token etc.
            $this->plugin->tv->load_channels();
        }

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) === "M3U") {
            // M3U search uses the indexes of the VOD playlist
            $category_list = null;
            $category_index = null;
            if (!$this->plugin->config->fetchVodCategories($category_list, $category_index)) {
                hd_debug_print("Error: Fetch categories");
                return Action_Factory::show_title_dialog(TR::t('err_load_vod'));
            }
        }

        return Starnet_Vod_List_Screen::make_search_action($this->plugin, $search_string);
    }

    private function get_resume_mediaurl()
    {
        $media_url = null;
        if (file_exists('/config/resume_state.properties')) {
            $resume_state = parse_ini_file('/config/resume_state.properties', 0, INI_SCANNER_RAW);

            if (strpos($resume_state['plugin_name'], get_plugin_name()) !== false) {
                $media_url = MediaURL::decode();
                $media_url->is_favorite = $resume_state['plugin_tv_is_favorite'];
                $media_url->group_id = $resume_state['plugin_tv_is_favorite'] ? FAVORITES_GROUP_ID : $resume_state['plugin_tv_group'];
                $media_url->channel_id = $resume_state['plugin_tv_channel'];
                $media_url->archive_tm = ((time() - $resume_state['plugin_tv_archive_tm']) < 259200) ? $resume_state['plugin_tv_archive_tm'] : -1;
            }
        }
        return $media_url;
    }
}

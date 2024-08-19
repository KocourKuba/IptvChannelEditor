<?php
/**
 * The MIT License (MIT)
 *
 * @Author: sharky72 (https://github.com/KocourKuba)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Ext_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'ext_setup';

    const CONTROL_HISTORY_CHANGE_FOLDER = 'change_history_folder';
    const CONTROL_COPY_TO_DATA = 'copy_to_data';
    const CONTROL_COPY_TO_PLUGIN = 'copy_to_plugin';
    const CONTROL_HISTORY_FOLDER = 'history_folder';
    const CONTROL_TV_HISTORY_CLEAR = 'history_clear_tv';
    const CONTROL_VOD_HISTORY_CLEAR = 'history_clear_vod';
    const CONTROL_HISTORY_CLEAR = 'history_clear_vod';
    const SETUP_ACTION_PASS_DLG = 'pass_dialog';
    const SETUP_ACTION_PASS_APPLY = 'pass_apply';

    ///////////////////////////////////////////////////////////////////////

    /**
     * defs for all controls on screen
     * @return array
     */
    public function do_get_control_defs()
    {
        hd_debug_print(null, true);

        $defs = array();

        $folder_icon = get_image_path('folder.png');
        $remove_icon = get_image_path('brush.png');
        $refresh_icon = get_image_path('refresh.png');
        $text_icon = get_image_path('text.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // history

        $history_path = $this->plugin->get_history_path();
        hd_debug_print("history path: $history_path");
        $display_path = HD::string_ellipsis($history_path);

        Control_Factory::add_image_button($defs, $this, null,
            self::CONTROL_HISTORY_CHANGE_FOLDER, TR::t('setup_history_folder_path'), $display_path, $folder_icon, self::CONTROLS_WIDTH);

        $path = $this->plugin->get_parameter(PARAM_HISTORY_PATH);
        if (!is_null($path) && $history_path !== get_data_path(Default_Dune_Plugin::HISTORY_FOLDER)) {
            Control_Factory::add_image_button($defs, $this, null,
                self::CONTROL_COPY_TO_DATA, TR::t('setup_copy_to_data'), TR::t('apply'), $refresh_icon, self::CONTROLS_WIDTH);

            Control_Factory::add_image_button($defs, $this, null,
                self::CONTROL_COPY_TO_PLUGIN, TR::t('setup_copy_to_plugin'), TR::t('apply'), $refresh_icon, self::CONTROLS_WIDTH);
        }

        Control_Factory::add_image_button($defs, $this, null,
            self::CONTROL_TV_HISTORY_CLEAR, TR::t('setup_tv_history_clear'), TR::t('clear'), $remove_icon, self::CONTROLS_WIDTH);

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) !== "None") {
            Control_Factory::add_image_button($defs, $this, null,
                self::CONTROL_VOD_HISTORY_CLEAR, TR::t('setup_vod_history_clear'), TR::t('clear'), $remove_icon, self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // https proxy settings
        if (is_updater_proxy_needs()) {
            $use_proxy = $this->plugin->get_parameter(PARAM_USE_UPDATER_PROXY, SetupControlSwitchDefs::switch_off);
            Control_Factory::add_image_button($defs, $this, null, PARAM_USE_UPDATER_PROXY,
                TR::t('setup_updater_proxy'), SetupControlSwitchDefs::$on_off_translated[$use_proxy],
                get_image_path(SetupControlSwitchDefs::$on_off_img[$use_proxy]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // adult channel password
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_PASS_DLG,
            TR::t('setup_adult_title'), TR::t('setup_adult_change'), $text_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // debugging

        $debug_state = $this->plugin->get_parameter(PARAM_ENABLE_DEBUG, SetupControlSwitchDefs::switch_off);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_ENABLE_DEBUG, TR::t('setup_debug'), SetupControlSwitchDefs::$on_off_translated[$debug_state],
            get_image_path(SetupControlSwitchDefs::$on_off_img[$debug_state]), self::CONTROLS_WIDTH);

        return $defs;
    }

    /**
     * @inheritDoc
     */
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        return $this->do_get_control_defs();
    }

    /**
     * adult pass dialog defs
     * @return array
     */
    public function do_get_pass_control_defs()
    {
        $defs = array();

        $pass1 = '';
        $pass2 = '';

        Control_Factory::add_vgap($defs, 20);

        Control_Factory::add_text_field($defs, $this, null, 'pass1', TR::t('setup_old_pass'),
            $pass1, 1, true, 0, 1, 500, 0);
        Control_Factory::add_text_field($defs, $this, null, 'pass2', TR::t('setup_new_pass'),
            $pass2, 1, true, 0, 1, 500, 0);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_PASS_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        $action_reload = User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);
        $control_id = $user_input->control_id;
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            hd_debug_print("Setup: changing $control_id value to $new_value");
        }

        switch ($control_id) {

            case self::CONTROL_HISTORY_CHANGE_FOLDER:
                $media_url_str = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'source_window_id' => static::ID,
                        'choose_folder' => array(
                            'action' => self::CONTROL_HISTORY_FOLDER,
                        ),
                        'allow_reset' => true,
                        'allow_network' => !is_limited_apk(),
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url_str, TR::t('setup_history_folder_path'));

            case self::CONTROL_HISTORY_FOLDER:
                $data = MediaURL::decode($user_input->selected_data);
                hd_debug_print(self::CONTROL_HISTORY_FOLDER . " $data->filepath");
                $this->plugin->set_parameter(PARAM_HISTORY_PATH, smb_tree::set_folder_info($user_input->selected_data));
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $data->caption),
                    $action_reload, $data->filepath, self::CONTROLS_WIDTH);

            case ACTION_RESET_DEFAULT:
                hd_debug_print("do set history folder to default");
                $this->plugin->remove_parameter(PARAM_HISTORY_PATH);
                return $action_reload;

            case self::CONTROL_COPY_TO_DATA:
                $history_path = $this->plugin->get_history_path();
                hd_debug_print("copy to: $history_path");
                if (!HD::copy_data(get_data_path(Default_Dune_Plugin::HISTORY_FOLDER),
                    "/" . PARAM_TV_HISTORY_ITEMS ."$/", $history_path)) {

                    return Action_Factory::show_title_dialog(TR::t('err_copy'));
                }

                return Action_Factory::show_title_dialog(TR::t('setup_copy_done'), $action_reload);

            case self::CONTROL_COPY_TO_PLUGIN:
                hd_debug_print("copy to: " . get_data_path(Default_Dune_Plugin::HISTORY_FOLDER));
                if (!HD::copy_data($this->plugin->get_history_path(),
                    "/" . PARAM_TV_HISTORY_ITEMS ."$/", get_data_path(Default_Dune_Plugin::HISTORY_FOLDER))) {
                    return Action_Factory::show_title_dialog(TR::t('err_copy'));
                }

                return Action_Factory::show_title_dialog(TR::t('setup_copy_done'), $action_reload);

            case PARAM_USE_UPDATER_PROXY:
                $old_val = $this->plugin->get_bool_parameter(PARAM_USE_UPDATER_PROXY, false);
                $use_proxy = $this->plugin->toggle_parameter(PARAM_USE_UPDATER_PROXY, false);
                if (!toggle_updater_proxy($use_proxy)) {
                    $this->plugin->set_bool_parameter(PARAM_USE_UPDATER_PROXY, $old_val);
                    return Action_Factory::show_title_dialog(TR::t('err_changes_failed'));
                }

                $msg = $use_proxy ? TR::t('setup_use_updater_proxy_enabled') : TR::t('setup_use_updater_proxy_disable');
                return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'), Action_Factory::restart(), $msg);

            case self::CONTROL_TV_HISTORY_CLEAR:
                $history_path = $this->plugin->get_history_path();
                hd_debug_print("do clear TV history in $history_path");
                $this->plugin->get_playback_points()->clear_points();

                return Action_Factory::show_title_dialog(TR::t('setup_history_cleared'), $action_reload);

            case self::CONTROL_VOD_HISTORY_CLEAR:
                $filename = Starnet_Vod::VOD_HISTORY_ITEMS . "_" . $this->plugin->config->get_vod_template_name();
                $history_path = $this->plugin->get_history_path($filename);

                hd_debug_print("do clear VOD history $history_path");
                exec("rm -rf $history_path");

                return Action_Factory::show_title_dialog(TR::t('setup_history_cleared'), $action_reload);

            case self::SETUP_ACTION_PASS_DLG: // show pass dialog
                $defs = $this->do_get_pass_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_adult_password'), $defs, true);

            case self::SETUP_ACTION_PASS_APPLY: // handle pass dialog result
                $pass = $this->plugin->get_parameter(PARAM_ADULT_PASSWORD);
                $need_reload = true;
                if ($user_input->pass1 !== $pass) {
                    $msg = TR::t('err_wrong_old_password');
                } else if (empty($user_input->pass2)) {
                    $this->plugin->set_parameter(PARAM_ADULT_PASSWORD, '');
                    $msg = TR::t('setup_pass_disabled');
                } else if ($user_input->pass1 !== $user_input->pass2) {
                    $this->plugin->set_parameter(PARAM_ADULT_PASSWORD, $user_input->pass2);
                    $msg = TR::t('setup_pass_changed');
                } else {
                    $msg = TR::t('setup_pass_not_changed');
                    $need_reload = false;
                }

                if ($need_reload) {
                    $this->plugin->tv->reload_channels();
                }

                return Action_Factory::show_title_dialog($msg,
                    Action_Factory::reset_controls($this->do_get_control_defs()));

            case PARAM_ENABLE_DEBUG:
                $debug = $this->plugin->toggle_parameter(PARAM_ENABLE_DEBUG, false);
                hd_debug_print("Debug logging: " . var_export($debug, true));
                set_debug_log($debug);
                break;

            case ACTION_RELOAD:
                hd_debug_print(ACTION_RELOAD);
                return Action_Factory::reset_controls($this->do_get_control_defs(),
                    User_Input_Handler_Registry::create_action_screen(Starnet_Tv_Rows_Screen::ID, ACTION_REFRESH_SCREEN));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs());
    }

    public static function CopyData($sourcePath, $destPath){
        if (empty($sourcePath) || empty($destPath)) {
            hd_debug_print("sourceDir = $sourcePath | destDir = $destPath");
            return false;
        }

        foreach (glob($sourcePath) as $file) {
            $dest_file = $destPath . basename($file);
            hd_debug_print("copy $file to $dest_file");
            if (!copy($file, $dest_file))
                return false;
        }
        return true;
    }
}

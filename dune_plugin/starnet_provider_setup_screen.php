<?php
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Provider_Setup_Screen extends Abstract_Controls_Screen
{
    const ID = 'provider_setup';

    const SETUP_ACTION_OTTKEY_DLG = 'ott_key_dialog';
    const SETUP_ACTION_OTTKEY_APPLY = 'ott_key_apply';
    const SETUP_ACTION_LOGIN_DLG = 'login_dialog';
    const SETUP_ACTION_LOGIN_APPLY = 'login_apply';
    const SETUP_ACTION_PIN_DLG = 'pin_dialog';
    const SETUP_ACTION_PIN_APPLY = 'pin_apply';
    const SETUP_ACTION_CLEAR_ACCOUNT = 'clear_account';
    const SETUP_ACTION_CLEAR_ACCOUNT_APPLY = 'clear_account_apply';

    const CONTROL_SERVER = 'server';
    const CONTROL_DEVICE = 'device';
    const CONTROL_PROFILE = 'profile';
    const CONTROL_QUALITY = 'quality';
    const CONTROL_DOMAIN = 'domain';
    const CONTROL_API_DOMAIN = 'api_domain';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs();
    }

    /**
     * streaming parameters dialog defs
     *
     * @return array
     */
    public function do_get_control_defs()
    {
        $defs = array();

        $delete_icon = get_image_path('brush.png');
        $text_icon = get_image_path('text.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // ott or token dialog
        if ($this->plugin->config->get_embedded_account() !== null) {
            Control_Factory::add_image_button($defs, $this, self::SETUP_ACTION_CLEAR_ACCOUNT, TR::t('setup_account_title'),
                TR::t('setup_account_delete'), $delete_icon);
        } else {
            switch ($this->plugin->config->get_feature(Plugin_Constants::ACCESS_TYPE)) {
                case Plugin_Constants::ACCOUNT_OTT_KEY:
                    Control_Factory::add_image_button($defs, $this, self::SETUP_ACTION_OTTKEY_DLG, TR::t('setup_account_title'),
                        TR::t('setup_account_ott'), $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_LOGIN:
                    Control_Factory::add_image_button($defs, $this, self::SETUP_ACTION_LOGIN_DLG, TR::t('setup_account_title'),
                        TR::t('setup_account_login'), $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_PIN:
                    Control_Factory::add_image_button($defs, $this, self::SETUP_ACTION_PIN_DLG, TR::t('setup_account_title'),
                        TR::t('setup_account_pin'), $text_icon);
                    break;
            }
        }

        //////////////////////////////////////
        // select server
        $servers = $this->plugin->config->get_servers();
        if (!empty($servers)) {
            $server_id = $this->plugin->config->get_server_id();
            $server_name = $this->plugin->config->get_server_name();
            hd_debug_print("Selected server: id: '$server_id' name: '$server_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_SERVER,
                TR::t('server'), $server_id, $servers);
        }

        //////////////////////////////////////
        // select device number
        $devices = $this->plugin->config->get_devices();
        if (!empty($devices)) {
            $device_id = $this->plugin->config->get_device_id();
            $device_name = $this->plugin->config->get_device_name();
            hd_debug_print("Selected device: id: '$device_id' name: '$device_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_DEVICE,
                TR::t('setup_device'), $device_id, $devices);
        }

        //////////////////////////////////////
        // select quality
        $qualities = $this->plugin->config->get_qualities();
        if (!empty($qualities)) {
            $quality_id = $this->plugin->config->get_quality_id();
            $quality_name = $this->plugin->config->get_quality_name();
            hd_debug_print("Selected quality: id: '$quality_id' name: '$quality_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_QUALITY,
                TR::t('setup_quality'), $quality_id, $qualities);
        }

        //////////////////////////////////////
        // select profile
        $profiles = $this->plugin->config->get_profiles();
        if (!empty($profiles)) {
            $profile_id = $this->plugin->config->get_profile_id();
            $profile_name = $this->plugin->config->get_profile_name();
            hd_debug_print("Selected profile: id: '$profile_id' name: '$profile_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_PROFILE,
                TR::t('setup_profile'), $profile_id, $profiles);
        }

        //////////////////////////////////////
        // select domain
        $domains = $this->plugin->config->get_domains();
        if (count($domains) > 1) {
            $domain_id = $this->plugin->config->get_domain_id();
            $domain_name = $this->plugin->config->get_domain_name();
            hd_debug_print("Selected domain: id: '$domain_id' name: '$domain_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_DOMAIN,
                TR::t('setup_domain'), $domain_id, $domains);
        }

        //////////////////////////////////////
        // select domain
        $api_domains = $this->plugin->config->get_api_domains();
        if (count($api_domains) > 1) {
            $api_domain_id = $this->plugin->config->get_api_domain_id();
            $api_domain_name = $this->plugin->config->get_api_domain_name();
            hd_debug_print("Selected domain: id: '$api_domain_id' name: '$api_domain_name'");
            Control_Factory::add_combobox($defs, $this, self::CONTROL_API_DOMAIN,
                TR::t('setup_domain'), $api_domain_id, $api_domains);
        }

        //////////////////////////////////////
        // select stream type
        $format_ops = array();
        if ($this->plugin->config->get_stream_parameter(Plugin_Constants::HLS, Stream_Params::URL_TEMPLATE) !== '') {
            $format_ops[Plugin_Constants::HLS] = 'HLS';
        }

        if ($this->plugin->config->get_stream_parameter(Plugin_Constants::MPEG, Stream_Params::URL_TEMPLATE) !== '') {
            $format_ops[Plugin_Constants::MPEG] = 'MPEG-TS';
        }

        if (count($format_ops) > 1) {
            $format_id = $this->plugin->config->get_format();
            hd_debug_print("Selected stream type: id: $format_id name: '$format_ops[$format_id]'");
            Control_Factory::add_combobox($defs, $this, PARAM_STREAM_FORMAT,
                TR::t('setup_stream'), $format_id, $format_ops);
        }

        return $defs;
    }

    /**
     * ott key dialog defs
     * @return array
     */
    public function do_get_ott_key_control_defs()
    {
        $defs = array();
        $ott_key = $this->plugin->get_credentials(Ext_Params::M_OTT_KEY);
        $subdomain = $this->plugin->get_credentials(Ext_Params::M_SUBDOMAIN);
        $vportal = $this->plugin->get_credentials(Ext_Params::M_VPORTAL);

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, 'subdomain', TR::t('setup_enter_domain'), $subdomain,
            false, false, false, true);

        Control_Factory::add_text_field($defs, $this, 'ott_key', TR::t('setup_enter_ott'), $ott_key,
            false, true, false, true);

        Control_Factory::add_text_field($defs, $this, 'vportal', TR::t('setup_enter_vportal'), $vportal,
            false, false, false, true);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, self::SETUP_ACTION_OTTKEY_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * login dialog defs
     * @return array
     */
    public function do_get_login_control_defs()
    {
        $defs = array();

        $login = $this->plugin->get_credentials(Ext_Params::M_LOGIN);

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, 'login', TR::t('login'), $login,
            false, false, false, true);

        $password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
        Control_Factory::add_text_field($defs, $this, 'password', TR::t('password'), $password,
            false, true, false, true);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, self::SETUP_ACTION_LOGIN_APPLY, TR::t('apply'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * token dialog defs
     * @return array
     */
    public function do_get_pin_control_defs()
    {
        $defs = array();

        $password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, 'password', TR::t('password'), $password,
            false, true, false, true);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, self::SETUP_ACTION_PIN_APPLY, TR::t('apply'), 300);
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

        $control_id = $user_input->control_id;
        $need_reload = false;
        switch ($control_id) {
            case GUI_EVENT_KEY_TOP_MENU:
            case GUI_EVENT_KEY_RETURN:
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                return self::make_return_action($parent_media_url);

            case self::CONTROL_SERVER:
            case self::CONTROL_DEVICE:
            case self::CONTROL_PROFILE:
            case self::CONTROL_QUALITY:
            case self::CONTROL_DOMAIN:
            case self::CONTROL_API_DOMAIN:
                $func_get_id = "get_{$control_id}_id";
                $func_get_name = "get_{$control_id}_name";
                $func_set_id = "set_{$control_id}_id";
                $curr_id = $this->plugin->config->{$func_get_id}();
                $curr_name = $this->plugin->config->{$func_get_name}();
                $selected_value = $user_input->{$control_id};
                hd_debug_print("selected $control_id id: $selected_value", true);
                hd_debug_print("current plugin $control_id id: '$curr_id' name: '$curr_name'", true);
                if ($curr_id !== $selected_value) {
                    $need_reload = true;
                    $this->plugin->config->{$func_set_id}($selected_value);
                    $new_id = $this->plugin->config->{$func_get_id}();
                    $new_name = $this->plugin->config->{$func_get_name}();
                    hd_debug_print("new plugin $control_id id: '$new_id' name: '$new_name'", true);
                }
                break;

            case PARAM_STREAM_FORMAT:
                $need_reload = true;
                $this->plugin->set_setting($control_id, $user_input->{$control_id});
                hd_debug_print("$control_id: " . $user_input->{$control_id}, true);
                break;

            case self::SETUP_ACTION_OTTKEY_DLG: // show ott key dialog
                $defs = $this->do_get_ott_key_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_enter_key'), $defs, true);

            case self::SETUP_ACTION_OTTKEY_APPLY: // handle ott key dialog result
                $this->plugin->set_credentials(Ext_Params::M_OTT_KEY, $user_input->ott_key);
                $this->plugin->set_credentials(Ext_Params::M_SUBDOMAIN, $user_input->subdomain);

                if (!empty($user_input->vportal) && !preg_match('/^portal::\[key:([^]]+)\](.+)$/', $user_input->vportal)) {
                    return Action_Factory::show_title_dialog(TR::t('setup_wrong_vportal'));
                }

                $this->plugin->set_credentials(Ext_Params::M_VPORTAL, $user_input->vportal);
                $need_reload = true;
                break;

            case self::SETUP_ACTION_LOGIN_DLG: // token dialog
                $defs = $this->do_get_login_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_enter_key'), $defs, true);

            case self::SETUP_ACTION_LOGIN_APPLY: // handle token dialog result
                $old_login = $this->plugin->get_credentials(Ext_Params::M_LOGIN);
                $old_password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
                $this->plugin->set_credentials(Ext_Params::M_LOGIN, $user_input->login);
                $this->plugin->set_credentials(Ext_Params::M_PASSWORD, $user_input->password);
                $account_data = $this->plugin->config->GetAccountInfo(true);
                if ($account_data === false) {
                    $this->plugin->set_credentials(Ext_Params::M_LOGIN, $old_login);
                    $this->plugin->set_credentials(Ext_Params::M_PASSWORD, $old_password);
                    return Action_Factory::show_title_dialog(TR::t('setup_wrong_pass'));
                }

                $need_reload = true;
                break;

            case self::SETUP_ACTION_PIN_DLG: // token dialog
                $defs = $this->do_get_pin_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_enter_key'),
                    $defs, true);

            case self::SETUP_ACTION_PIN_APPLY: // handle token dialog result
                $old_password = $this->plugin->get_credentials(Ext_Params::M_PASSWORD);
                $this->plugin->set_credentials(Ext_Params::M_PASSWORD, $user_input->password);
                $account_data = $this->plugin->config->GetAccountInfo(true);
                if ($account_data === false) {
                    $this->plugin->set_credentials(Ext_Params::M_PASSWORD, $old_password);
                    return Action_Factory::show_title_dialog(TR::t('setup_wrong_pass'));
                }

                $need_reload = true;
                break;

            case self::SETUP_ACTION_CLEAR_ACCOUNT: // confirm clear account
                if ($this->plugin->config->get_embedded_account() === null) break;

                return Action_Factory::show_confirmation_dialog(TR::t('warning'), $this, self::SETUP_ACTION_CLEAR_ACCOUNT_APPLY,
                    TR::t('setup_delete_embedded'));

            case self::SETUP_ACTION_CLEAR_ACCOUNT_APPLY: // handle clear account
                exec('rm -rf ' . get_install_path('account.dat'));
                exec('rm -rf ' . get_data_path('account.dat'));
                $this->plugin->config->set_embedded_account(null);
                $post_action = User_Input_Handler_Registry::create_action($this, RESET_CONTROLS_ACTION_ID);
                return Action_Factory::show_title_dialog(TR::t('setup_deleted_embedded'), '', $post_action);
        }

        if ($need_reload) {
            $this->plugin->tv->reload_channels();
            return Action_Factory::invalidate_all_folders($plugin_cookies, Action_Factory::reset_controls($this->do_get_control_defs()));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs());
    }
}

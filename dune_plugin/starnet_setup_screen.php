<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'setup';

    const SETUP_ACTION_OTTKEY_DLG = 'ott_key_dialog';
    const SETUP_ACTION_OTTKEY_APPLY = 'ott_key_apply';
    const SETUP_ACTION_LOGIN_DLG = 'login_dialog';
    const SETUP_ACTION_LOGIN_APPLY = 'login_apply';
    const SETUP_ACTION_PIN_DLG = 'pin_dialog';
    const SETUP_ACTION_PIN_APPLY = 'pin_apply';
    const SETUP_ACTION_CLEAR_ACCOUNT = 'clear_account';
    const SETUP_ACTION_CLEAR_ACCOUNT_APPLY = 'clear_account_apply';
    const SETUP_ACTION_STRIP_HTTPS = 'strip_https';
    const SETUP_ACTION_PASS_DLG = 'pass_dialog';
    const SETUP_ACTION_PASS_APPLY = 'pass_apply';
    const SETUP_ACTION_USE_HTTPS_PROXY = 'use_proxy';

    const SETUP_ACTION_INTERFACE_SCREEN = 'interface_screen';
    const SETUP_ACTION_CHANNELS_SCREEN = 'channels_screen';
    const SETUP_ACTION_EPG_SCREEN = 'epg_screen';
    const SETUP_ACTION_STREAMING_SCREEN = 'streaming_screen';
    const SETUP_ACTION_HISTORY_SCREEN = 'history_screen';

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_on => '%tr%yes',
        SetupControlSwitchDefs::switch_off => '%tr%no',
        SetupControlSwitchDefs::switch_small => '%tr%setup_small',
        SetupControlSwitchDefs::switch_normal => '%tr%setup_normal',
        SetupControlSwitchDefs::switch_epg1 => '%tr%setup_first',
        SetupControlSwitchDefs::switch_epg2 => '%tr%setup_second',
    );

    private static $on_off_img = array
    (
        SetupControlSwitchDefs::switch_on => 'on.png',
        SetupControlSwitchDefs::switch_off => 'off.png',
        SetupControlSwitchDefs::switch_small => 'on.png',
        SetupControlSwitchDefs::switch_normal => 'off.png',
        SetupControlSwitchDefs::switch_epg1 => 'off.png',
        SetupControlSwitchDefs::switch_epg2 => 'on.png',
    );

    ///////////////////////////////////////////////////////////////////////

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin);

        $plugin->create_screen($this);
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * defs for all controls on screen
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_control_defs(&$plugin_cookies)
    {
        $delete_icon = $this->plugin->get_image_path('brush.png');
        $text_icon = $this->plugin->get_image_path('text.png');
        $setting_icon = $this->plugin->get_image_path('settings.png');

        $defs = array();
        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // ott or token dialog
        if ($this->plugin->config->get_embedded_account() !== null) {
            Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CLEAR_ACCOUNT,
                TR::t('setup_account_title'), TR::t('setup_account_delete'), $delete_icon);
        } else {
            switch ($this->plugin->config->get_feature(Plugin_Constants::ACCESS_TYPE)) {
                case Plugin_Constants::ACCOUNT_OTT_KEY:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_OTTKEY_DLG,
                        TR::t('setup_account_title'), TR::t('setup_account_ott'), $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_LOGIN:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_LOGIN_DLG,
                        TR::t('setup_account_title'), TR::t('setup_account_login'), $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_PIN:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_PIN_DLG,
                        TR::t('setup_account_title'), TR::t('setup_account_pin'), $text_icon);
                    break;
            }
        }

        //////////////////////////////////////
        // Interface settings
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_INTERFACE_SCREEN,
            TR::t('setup_interface_title'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Channels settings
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_SCREEN,
            TR::t('tv_screen_channels_setup'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // EPG settings
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_EPG_SCREEN,
            TR::t('setup_epg_settings'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Streaming settings
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_STREAMING_SCREEN,
            TR::t('setup_streaming_settings'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // History view info location
        if (!is_apk()) {
            Control_Factory::add_image_button($defs, $this, null,
                self::SETUP_ACTION_HISTORY_SCREEN,
                TR::t('setup_history_folder_path'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // https proxy settings
        if ((is_update_url_https() || is_https_proxy_enabled())
            && strpos(get_platform_kind(), '86') !== 0) {

            $use_proxy = isset($plugin_cookies->use_proxy) ? $plugin_cookies->use_proxy : SetupControlSwitchDefs::switch_off;
            Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_USE_HTTPS_PROXY,
                TR::t('setup_https_proxy'), self::$on_off_ops[$use_proxy],
                $this->plugin->get_image_path(self::$on_off_img[$use_proxy]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // adult channel password
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_PASS_DLG,
            TR::t('setup_adult_title'), TR::t('setup_adult_change'), $text_icon, self::CONTROLS_WIDTH);

        return $defs;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs($plugin_cookies);
    }

    /**
     * ott key dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_ott_key_control_defs(&$plugin_cookies)
    {
        $defs = array();
        $ott_key = isset($plugin_cookies->ott_key) ? $plugin_cookies->ott_key : '';
        $subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : '';
        $vportal = isset($plugin_cookies->mediateka) ? $plugin_cookies->mediateka : '';

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, 'subdomain', TR::t('setup_enter_domain'),
            $subdomain, false, false, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'ott_key', TR::t('setup_enter_ott'),
            $ott_key, false, true, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'vportal', TR::t('setup_enter_vportal'),
            $vportal, false, false, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_OTTKEY_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * login dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_login_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $login = isset($plugin_cookies->login) ? $plugin_cookies->login : '';

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, 'login', TR::t('login'),
            $login, false, false, false, true, 600);

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
        Control_Factory::add_text_field($defs, $this, null, 'password', TR::t('password'),
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_LOGIN_APPLY, TR::t('apply'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * token dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_pin_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, 'password', TR::t('password'),
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_PIN_APPLY, TR::t('apply'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
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
     * user remote input handler Implementation of UserInputHandler
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        $control_id = $user_input->control_id;
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            //hd_print("Setup: changing $control_id value to $new_value");
        }

        $need_reload = false;
        switch ($control_id) {

            case self::SETUP_ACTION_OTTKEY_DLG: // show ott key dialog
                $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_enter_key'), $defs, true);

            case self::SETUP_ACTION_OTTKEY_APPLY: // handle ott key dialog result
                $plugin_cookies->ott_key = $user_input->ott_key;
                $plugin_cookies->subdomain = $user_input->subdomain;
                $plugin_cookies->mediateka = $user_input->vportal;
                $need_reload = true;
                break;

            case self::SETUP_ACTION_LOGIN_DLG: // token dialog
                $defs = $this->do_get_login_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_enter_key'), $defs, true);

            case self::SETUP_ACTION_LOGIN_APPLY: // handle token dialog result
                $old_login = isset($plugin_cookies->login) ? $plugin_cookies->login : '';
                $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                $plugin_cookies->login = $user_input->login;
                $plugin_cookies->password = $user_input->password;
                $account_data = $this->plugin->config->GetAccountInfo($plugin_cookies, true);
                if ($account_data === false) {
                    $plugin_cookies->login = $old_login;
                    $plugin_cookies->password = $old_password;
                    return Action_Factory::show_title_dialog(TR::t('setup_wrong_pass'));
                }

                $need_reload = true;
                break;

            case self::SETUP_ACTION_PIN_DLG: // token dialog
                $defs = $this->do_get_pin_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_enter_key'),
                    $defs, true);

            case self::SETUP_ACTION_PIN_APPLY: // handle token dialog result
                $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                $plugin_cookies->password = $user_input->password;
                $account_data = $this->plugin->config->GetAccountInfo($plugin_cookies, true);
                if ($account_data === false) {
                    $plugin_cookies->password = $old_password;
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
                return Action_Factory::show_title_dialog(TR::t('setup_deleted_embedded'), $post_action);

            case self::SETUP_ACTION_INTERFACE_SCREEN: // show interface settings dialog
                return Action_Factory::open_folder(Starnet_Interface_Setup_Screen::get_media_url_str(), TR::t('setup_interface_title'));

            case self::SETUP_ACTION_CHANNELS_SCREEN: // show epg settings dialog
                return Action_Factory::open_folder(Starnet_Channels_Setup_Screen::get_media_url_str(), TR::t('tv_screen_channels_setup'));

            case self::SETUP_ACTION_EPG_SCREEN: // show epg settings dialog
                return Action_Factory::open_folder(Starnet_Epg_Setup_Screen::get_media_url_str(), TR::t('setup_epg_settings'));

            case self::SETUP_ACTION_STREAMING_SCREEN: // show streaming settings dialog
                return Action_Factory::open_folder(Starnet_Streaming_Setup_Screen::get_media_url_str(), TR::t('setup_streaming_settings'));

            case self::SETUP_ACTION_HISTORY_SCREEN:
                return Action_Factory::open_folder(Starnet_History_Setup_Screen::get_media_url_str(), TR::t('setup_history_change_folder'));

            case self::SETUP_ACTION_USE_HTTPS_PROXY:
                $old_value = $plugin_cookies->{$control_id};
                $plugin_cookies->use_proxy = ($plugin_cookies->{$control_id} === SetupControlSwitchDefs::switch_on)
                    ? SetupControlSwitchDefs::switch_off
                    : SetupControlSwitchDefs::switch_on;
                if (HD::toggle_https_proxy($plugin_cookies) === 0) {
                    $plugin_cookies->{$control_id} = $old_value;
                    return Action_Factory::show_title_dialog(TR::t('err_changes_failed'));
                }

                $msg = ($plugin_cookies->{$control_id} === SetupControlSwitchDefs::switch_on
                    ? TR::t('setup_use_https_proxy_enabled')
                    : TR::t('setup_use_https_proxy_disabled'));
                return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'), Action_Factory::restart(), $msg);

            case self::SETUP_ACTION_PASS_DLG: // show pass dialog
                $defs = $this->do_get_pass_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_adult_password'), $defs, true);

            case self::SETUP_ACTION_PASS_APPLY: // handle pass dialog result
                if ($user_input->pass1 !== $plugin_cookies->pass_sex) {
                    $msg = TR::t('err_wrong_old_password');
                } else if (empty($user_input->pass2)) {
                    $plugin_cookies->pass_sex = '';
                    $msg = TR::t('setup_pass_disabled');
                    $need_reload = true;
                } else if ($user_input->pass1 !== $user_input->pass2) {
                    $plugin_cookies->pass_sex = $user_input->pass2;
                    $msg = TR::t('setup_pass_changed');
                    $need_reload = true;
                } else {
                    $msg = TR::t('setup_pass_not_changed');
                }

                return Action_Factory::show_title_dialog($msg, $need_reload ? $this->plugin->tv->reload_channels($this, $plugin_cookies) : null);
        }

        if ($need_reload) {
            return $this->plugin->tv->reload_channels($this, $plugin_cookies);
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

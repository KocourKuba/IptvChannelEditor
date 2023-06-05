<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_SHOW_TV = 'show_tv';
    const SETUP_ACTION_VOD_LAST = 'vod_last';
    const SETUP_ACTION_SHOW_ALL = 'show_all';
    const SETUP_ACTION_OTTKEY_DLG = 'ott_key_dialog';
    const SETUP_ACTION_OTTKEY_APPLY = 'ott_key_apply';
    const SETUP_ACTION_LOGIN_DLG = 'login_dialog';
    const SETUP_ACTION_LOGIN_APPLY = 'login_apply';
    const SETUP_ACTION_PIN_DLG = 'pin_dialog';
    const SETUP_ACTION_PIN_APPLY = 'pin_apply';
    const SETUP_ACTION_CLEAR_ACCOUNT = 'clear_account';
    const SETUP_ACTION_CLEAR_ACCOUNT_APPLY = 'clear_account_apply';
    const SETUP_ACTION_INTERFACE_DLG = 'interface_dialog';
    const SETUP_ACTION_INTERFACE_APPLY = 'interface_apply';
    const SETUP_ACTION_STREAMING_DLG = 'streaming_dialog';
    const SETUP_ACTION_STREAMING_APPLY = 'streaming_apply';
    const SETUP_ACTION_AUTO_RESUME = 'auto_resume';
    const SETUP_ACTION_AUTO_PLAY = 'auto_play';
    const SETUP_ACTION_STRIP_HTTPS = 'strip_https';
    const SETUP_ACTION_EPG_APPLY = 'epg_dialog_apply';
    const SETUP_ACTION_CLEAR_EPG_CACHE = 'clear_epg_cache';
    const SETUP_ACTION_PASS_DLG = 'pass_dialog';
    const SETUP_ACTION_PASS_APPLY = 'pass_apply';
    const SETUP_ACTION_USE_HTTPS_PROXY = 'use_proxy';

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
        $defs = array();

        $text_icon = $this->plugin->get_image_path('text.png');
        $setting_icon = $this->plugin->get_image_path('settings.png');

        //////////////////////////////////////
        // Plugin name
        Control_Factory::add_vgap($defs, -10);
        $title = " v.{$this->plugin->config->plugin_info['app_version']} [{$this->plugin->config->plugin_info['app_release_date']}]";
        Control_Factory::add_label($defs, "IPTV Channel Editor by sharky72", $title, 20);

        //////////////////////////////////////
        // ott or token dialog
        if ($this->plugin->config->get_embedded_account() !== null) {
            Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CLEAR_ACCOUNT,
                TR::t('setup_account_title'), TR::t('setup_account_delete'), $setting_icon);
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
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_INTERFACE_DLG,
            TR::t('setup_interface_title'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // streaming dialog
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_STREAMING_DLG,
            TR::t('setup_epg_settings'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

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

        Control_Factory::add_vgap($defs, 10);

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
     * interface dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_interface_control_defs(&$plugin_cookies)
    {
        hd_print(__METHOD__);
        $defs = array();
        Control_Factory::add_vgap($defs, 20);

        $on_off = array();
        $on_off[SetupControlSwitchDefs::switch_on] = self::$on_off_ops[SetupControlSwitchDefs::switch_on];
        $on_off[SetupControlSwitchDefs::switch_off] = self::$on_off_ops[SetupControlSwitchDefs::switch_off];

        //////////////////////////////////////
        // Show in main screen
        if (!is_apk()) {
            $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : SetupControlSwitchDefs::switch_on;
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_SHOW_TV, TR::t('setup_show_in_main'), $show_tv, $on_off, 0);
        }

        //////////////////////////////////////
        // show all channels category
        $show_all = isset($plugin_cookies->show_all) ? $plugin_cookies->show_all : SetupControlSwitchDefs::switch_on;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_SHOW_ALL, TR::t('setup_show_all_channels'), $show_all, $on_off, 0);

        //////////////////////////////////////
        // show vod at the end of categories
        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $vod_last = isset($plugin_cookies->vod_last) ? $plugin_cookies->vod_last : SetupControlSwitchDefs::switch_on;
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_VOD_LAST, TR::t('setup_vod_last'), $vod_last, $on_off, 0);
        }

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_INTERFACE_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
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
     * streaming parameters dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_streaming_control_defs(&$plugin_cookies)
    {
        hd_print(__METHOD__);
        $defs = array();
        Control_Factory::add_vgap($defs, 20);

        //////////////////////////////////////
        // auto play
        $on_off = array();
        $on_off[SetupControlSwitchDefs::switch_on] = self::$on_off_ops[SetupControlSwitchDefs::switch_on];
        $on_off[SetupControlSwitchDefs::switch_off] = self::$on_off_ops[SetupControlSwitchDefs::switch_off];
        $auto_play = isset($plugin_cookies->auto_play) ? $plugin_cookies->auto_play : SetupControlSwitchDefs::switch_off;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_AUTO_PLAY, TR::t('setup_autostart'), $auto_play, $on_off, 0);

        //////////////////////////////////////
        // auto resume
        $auto_resume = isset($plugin_cookies->auto_resume) ? $plugin_cookies->auto_resume : SetupControlSwitchDefs::switch_on;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_AUTO_RESUME, TR::t('setup_continue_play'), $auto_resume, $on_off, 0);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_button($defs, $this, null, self::SETUP_ACTION_CLEAR_EPG_CACHE, TR::t('entry_epg_cache_clear'), TR::t('clear'), 0);

        //////////////////////////////////////
        // EPG
        $epg_params = $this->plugin->config->get_epg_params(Plugin_Constants::EPG_SECOND);
        if (!empty($epg_params[Epg_Params::EPG_URL])) {
            $epg_source_ops = array();
            $epg_source_ops[SetupControlSwitchDefs::switch_epg1] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg1];
            $epg_source_ops[SetupControlSwitchDefs::switch_epg2] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg2];

            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;

            Control_Factory::add_combobox($defs, $this, null,
                'epg_source', TR::t('setup_epg_source'), $epg_source, $epg_source_ops, 0);
        }

        $epg_font_ops = array();
        $epg_font_ops[SetupControlSwitchDefs::switch_small] = self::$on_off_ops[SetupControlSwitchDefs::switch_small];
        $epg_font_ops[SetupControlSwitchDefs::switch_normal] = self::$on_off_ops[SetupControlSwitchDefs::switch_normal];

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;

        Control_Factory::add_combobox($defs, $this, null,
            'epg_font_size', TR::t('setup_epg_font'), $epg_font_size, $epg_font_ops, 0);

        //////////////////////////////////////
        // select server
        $servers = $this->plugin->config->get_servers($plugin_cookies);
        if (!empty($servers)) {
            hd_print("Change server supported");
            $server_id = $this->plugin->config->get_server_id($plugin_cookies);
            $server_name = $this->plugin->config->get_server_name($plugin_cookies);
            hd_print("Selected server: id: $server_id name: '$server_name'");
            Control_Factory::add_combobox($defs, $this, null, 'server', TR::t('server'), $server_id, $servers, 0);
        }

        //////////////////////////////////////
        // select device number
        $devices = $this->plugin->config->get_devices($plugin_cookies);
        if (!empty($devices)) {
            hd_print("Change device supported");
            $device_id = $this->plugin->config->get_device_id($plugin_cookies);
            $device_name = $this->plugin->config->get_device_name($plugin_cookies);
            hd_print("Selected device: id: $device_id name: '$device_name'");
            Control_Factory::add_combobox($defs, $this, null, 'device', TR::t('setup_device'), $device_id, $devices, 0);
        }

        //////////////////////////////////////
        // select quality
        $qualities = $this->plugin->config->get_qualities($plugin_cookies);
        if (!empty($qualities)) {
            hd_print("Change quality supported");
            $quality_id = $this->plugin->config->get_quality_id($plugin_cookies);
            $quality_name = $this->plugin->config->get_quality_name($plugin_cookies);
            hd_print("Selected quality: id: $quality_id name: '$quality_name'");
            Control_Factory::add_combobox($defs, $this, null, 'quality', TR::t('setup_quality'), $quality_id, $qualities, 0);
        }

        //////////////////////////////////////
        // select profile
        $profiles = $this->plugin->config->get_profiles($plugin_cookies);
        if (!empty($profiles)) {
            hd_print("Change profile supported");
            $profile_id = $this->plugin->config->get_profile_id($plugin_cookies);
            $profile_name = $this->plugin->config->get_profile_name($plugin_cookies);
            hd_print("Selected profile: id: $profile_id name: '$profile_name'");
            Control_Factory::add_combobox($defs, $this, null, 'profile', TR::t('setup_profile'), $profile_id, $profiles, 0);
        }
        //////////////////////////////////////
        // select stream type
        $format_ops = array();
        if ($this->plugin->config->get_stream_param(Plugin_Constants::HLS, Stream_Params::URL_TEMPLATE) !== '') {
            $format_ops[Plugin_Constants::HLS] = 'HLS';
        }

        if ($this->plugin->config->get_stream_param(Plugin_Constants::MPEG, Stream_Params::URL_TEMPLATE) !== '') {
            $format_ops[Plugin_Constants::MPEG] = 'MPEG-TS';
        }

        if (count($format_ops) > 1) {
            hd_print("Change stream type supported");
            $format_id = $this->plugin->config->get_format($plugin_cookies);
            hd_print("Selected stream type: id: $format_id name: '$format_ops[$format_id]'");
            Control_Factory::add_combobox($defs, $this, null, 'stream_format', TR::t('setup_stream'), $format_id, $format_ops, 0);
        }

        //////////////////////////////////////
        // buffering time
        $show_buf_time_ops = array();
        $show_buf_time_ops[1000] = TR::t('setup_buffer_sec_default__1', "1");
        $show_buf_time_ops[0] = TR::t('setup_buffer_no');
        $show_buf_time_ops[500] = TR::t('setup_buffer_sec__1', "0.5");
        $show_buf_time_ops[2000] = TR::t('setup_buffer_sec__1', "2");
        $show_buf_time_ops[3000] = TR::t('setup_buffer_sec__1', "3");
        $show_buf_time_ops[5000] = TR::t('setup_buffer_sec__1', "5");
        $show_buf_time_ops[10000] = TR::t('setup_buffer_sec__1', "10");

        $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
        Control_Factory::add_combobox($defs, $this, null, 'buf_time', TR::t('setup_buffer_time'), $buf_time, $show_buf_time_ops, 0);

        //////////////////////////////////////
        // archive delay time
        $show_delay_time_ops = array();
        $show_delay_time_ops[60] = TR::t('setup_buffer_sec_default__1', "60");
        $show_delay_time_ops[10] = TR::t('setup_buffer_sec__1', "10");
        $show_delay_time_ops[20] = TR::t('setup_buffer_sec__1', "20");
        $show_delay_time_ops[30] = TR::t('setup_buffer_sec__1', "30");
        $show_delay_time_ops[2*60] = TR::t('setup_buffer_sec__1', "120");
        $show_delay_time_ops[3*60] = TR::t('setup_buffer_sec__1', "180");
        $show_delay_time_ops[5*60] = TR::t('setup_buffer_sec__1', "300");

        $delay_time = isset($plugin_cookies->delay_time) ? $plugin_cookies->delay_time : 60;
        Control_Factory::add_combobox($defs, $this, null, 'delay_time', TR::t('setup_delay_time'), $delay_time, $show_delay_time_ops, 0);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_STREAMING_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * EPG dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_epg_control_defs(&$plugin_cookies)
    {
        $defs = array();

        Control_Factory::add_vgap($defs, 20);
        $epg_params = $this->plugin->config->get_epg_params(Plugin_Constants::EPG_SECOND);
        if (!empty($epg_params[Epg_Params::EPG_URL])) {
            $epg_source_ops = array();
            $epg_source_ops[SetupControlSwitchDefs::switch_epg1] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg1];
            $epg_source_ops[SetupControlSwitchDefs::switch_epg2] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg2];

            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;

            Control_Factory::add_combobox($defs, $this, null,
                'epg_source', TR::t('setup_epg_source'), $epg_source, $epg_source_ops, 0);
        }

        $epg_font_ops = array();
        $epg_font_ops[SetupControlSwitchDefs::switch_small] = self::$on_off_ops[SetupControlSwitchDefs::switch_small];
        $epg_font_ops[SetupControlSwitchDefs::switch_normal] = self::$on_off_ops[SetupControlSwitchDefs::switch_normal];

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;

        Control_Factory::add_combobox($defs, $this, null,
            'epg_font_size', TR::t('setup_epg_font'), $epg_font_size, $epg_font_ops, 0);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_button($defs, $this, null,
            self::SETUP_ACTION_CLEAR_EPG_CACHE, TR::t('entry_epg_cache_clear'), TR::t('clear'), 0);

        Control_Factory::add_vgap($defs, 50);
        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_EPG_APPLY, TR::t('apply'), 300);
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
            hd_print("Setup: changing $control_id value to $new_value");
        }

        switch ($control_id) {

            case self::SETUP_ACTION_INTERFACE_DLG: // show interface settings dialog
                $defs = $this->do_get_interface_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_interface_title'), $defs, true);

            case self::SETUP_ACTION_INTERFACE_APPLY: // show interface settings dialog
                if (!is_apk()) {
                    $plugin_cookies->show_tv = $user_input->show_tv;
                    hd_print("Show on main screen: $plugin_cookies->show_tv");
                }

                $plugin_cookies->show_all = $user_input->show_all;
                hd_print("Show all channels category: $plugin_cookies->show_all");

                $plugin_cookies->vod_last = $user_input->vod_last;
                hd_print("Vod at last: $plugin_cookies->vod_last");

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

            case self::SETUP_ACTION_OTTKEY_DLG: // show ott key dialog
                $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_enter_key'), $defs, true);

            case self::SETUP_ACTION_OTTKEY_APPLY: // handle ott key dialog result
                $plugin_cookies->ott_key = $user_input->ott_key;
                $plugin_cookies->subdomain = $user_input->subdomain;
                $plugin_cookies->mediateka = $user_input->vportal;
                hd_print("portal info: $plugin_cookies->mediateka");
                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

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

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

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

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

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

            case self::SETUP_ACTION_STREAMING_DLG: // show streaming settings dialog
                $defs = $this->do_get_streaming_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_play_setup'), $defs, true);

            case self::SETUP_ACTION_STREAMING_APPLY: // handle streaming settings dialog result
                $plugin_cookies->auto_play = $user_input->auto_play;
                hd_print("Auto play: $plugin_cookies->auto_play");

                $plugin_cookies->auto_resume = $user_input->auto_resume;
                hd_print("Auto resume: $plugin_cookies->auto_resume");

                if (isset($user_input->epg_source)) {
                    $plugin_cookies->epg_source = $user_input->epg_source;
                    hd_print("Selected epg source: $user_input->epg_source");
                }
                $plugin_cookies->epg_font_size = $user_input->epg_font_size;
                hd_print("Selected epg font size: $user_input->epg_font_size");

                $need_reload = false;
                if (isset($user_input->stream_format)) {
                    $need_reload = true;
                    $plugin_cookies->format = $user_input->stream_format;
                    hd_print("selected stream type: $plugin_cookies->format");
                }

                if (isset($user_input->server) && $this->plugin->config->get_server_id($plugin_cookies) !== $user_input->server) {
                    $need_reload = true;
                    $this->plugin->config->set_server_id($user_input->server, $plugin_cookies);
                    hd_print("Selected server: id: $user_input->server name: '" . $this->plugin->config->get_server_name($plugin_cookies) . "'");
                }

                if (isset($user_input->quality) && $this->plugin->config->get_quality_id($plugin_cookies) !== $user_input->quality) {
                    $need_reload = true;
                    $this->plugin->config->set_quality_id($user_input->quality, $plugin_cookies);
                    hd_print("Selected quality: id: $user_input->quality name: '" . $this->plugin->config->get_quality_name($plugin_cookies) . "'");
                }

                if (isset($user_input->device) && $this->plugin->config->get_device_id($plugin_cookies) !== $user_input->device) {
                    $need_reload = true;
                    $this->plugin->config->set_device_id($user_input->device, $plugin_cookies);
                    hd_print("Selected device: id: $user_input->device name: '" . $this->plugin->config->get_device_name($plugin_cookies) . "'");
                }

                if (isset($user_input->profile) && $this->plugin->config->get_profile_id($plugin_cookies) !== $user_input->profile) {
                    $need_reload = true;
                    $this->plugin->config->set_profile_id($user_input->profile, $plugin_cookies);
                    hd_print("Selected profile: id: $user_input->profile name: '" . $this->plugin->config->get_profile_name($plugin_cookies) . "'");
                }

                $plugin_cookies->buf_time = $user_input->buf_time;
                hd_print("Buffering time: $plugin_cookies->buf_time");

                $plugin_cookies->delay_time = $user_input->delay_time;
                hd_print("Delay time: $plugin_cookies->delay_time");

                return $need_reload ? $this->plugin->tv->reload_channels($this, $plugin_cookies) : null;

            case self::SETUP_ACTION_CLEAR_EPG_CACHE: // clear epg cache
                $epg_path = get_temp_path("epg/");
                hd_print("do clear epg: $epg_path");
                foreach(glob($epg_path . "*") as $file) {
                    if(is_file($file)) {
                        hd_print("erase: $file");
                        unlink($file);
                    }
                }

                return Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));

            case self::SETUP_ACTION_USE_HTTPS_PROXY:
                $old_value = $plugin_cookies->use_proxy;
                $plugin_cookies->use_proxy = ($plugin_cookies->use_proxy === SetupControlSwitchDefs::switch_on)
                    ? SetupControlSwitchDefs::switch_off
                    : SetupControlSwitchDefs::switch_on;
                if (HD::toggle_https_proxy($plugin_cookies) === 0) {
                    $plugin_cookies->use_proxy = $old_value;
                    return Action_Factory::show_title_dialog(TR::t('err_changes_failed'));
                }

                $msg = ($plugin_cookies->use_proxy === SetupControlSwitchDefs::switch_on ? TR::t('setup_use_https_proxy_enabled') : TR::t('setup_use_https_proxy_disabled'));
                return Action_Factory::show_title_dialog(TR::t('entry_reboot_need'), Action_Factory::restart(), $msg);

            case self::SETUP_ACTION_PASS_DLG: // show pass dialog
                $defs = $this->do_get_pass_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_adult_password'), $defs, true);

            case self::SETUP_ACTION_PASS_APPLY: // handle pass dialog result
                $need_reload = false;
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

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

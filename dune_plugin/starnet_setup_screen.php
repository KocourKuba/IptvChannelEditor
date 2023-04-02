<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_SHOW_TV = 'show_tv';
    const SETUP_ACTION_OTTKEY_DLG = 'ott_key_dialog';
    const SETUP_ACTION_OTTKEY_APPLY = 'ott_key_apply';
    const SETUP_ACTION_LOGIN_DLG = 'login_dialog';
    const SETUP_ACTION_LOGIN_APPLY = 'login_apply';
    const SETUP_ACTION_PIN_DLG = 'pin_dialog';
    const SETUP_ACTION_PIN_APPLY = 'pin_apply';
    const SETUP_ACTION_CLEAR_ACCOUNT = 'clear_account';
    const SETUP_ACTION_CLEAR_ACCOUNT_APPLY = 'clear_account_apply';
    const SETUP_ACTION_CHANGE_LIST_PATH = 'change_list_path';
    const SETUP_ACTION_CHANGE_LIST = 'change_channels_list';
    const SETUP_ACTION_CHANNELS_SOURCE = 'channels_source';
    const SETUP_ACTION_CHANNELS_URL_DLG = 'channels_url_dialog';
    const SETUP_ACTION_CHANNELS_URL_APPLY = 'channels_url_apply';
    const SETUP_ACTION_VOD_LIST = 'vod_list';
    const SETUP_ACTION_CHANGE_VOD_LIST = 'change_vod_list';
    const SETUP_ACTION_STREAMING_DLG = 'streaming_dialog';
    const SETUP_ACTION_STREAMING_APPLY = 'streaming_apply';
    const SETUP_ACTION_AUTO_RESUME = 'auto_resume';
    const SETUP_ACTION_AUTO_PLAY = 'auto_play';
    const SETUP_ACTION_STRIP_HTTPS = 'strip_https';
    const SETUP_ACTION_EPG_APPLY = 'epg_dialog_apply';
    const SETUP_ACTION_CLEAR_EPG_CACHE = 'clear_epg_cache';
    const SETUP_ACTION_PASS_DLG = 'pass_dialog';
    const SETUP_ACTION_PASS_APPLY = 'pass_apply';
    const SETUP_ACTION_SEND_LOG = 'send_log';

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_on => 'Да',
        SetupControlSwitchDefs::switch_off => 'Нет',
        SetupControlSwitchDefs::switch_small => 'Мелкий',
        SetupControlSwitchDefs::switch_normal => 'Обычный',
        SetupControlSwitchDefs::switch_epg1 => 'Первичный',
        SetupControlSwitchDefs::switch_epg2 => 'Вторичный',
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

        //////////////////////////////////////
        // Plugin name
        Control_Factory::add_vgap($defs, -10);
        $title = " v.{$this->plugin->plugin_info['app_version']} [{$this->plugin->plugin_info['app_release_date']}]";
        Control_Factory::add_label($defs, "IPTV Channel Editor by sharky72", $title);

        $text_icon = $this->plugin->get_image_path('text.png');
        $folder_icon = $this->plugin->get_image_path('folder.png');
        $setting_icon = $this->plugin->get_image_path('settings.png');
        $web_icon = $this->plugin->get_image_path('web.png');
        $link_icon = $this->plugin->get_image_path('link.png');

        //////////////////////////////////////
        // Show in main screen
        if (!is_apk()) {
            $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
            Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_SHOW_TV, 'Показывать в главном меню:',
                self::$on_off_ops[$show_tv], $this->plugin->get_image_path(self::$on_off_img[$show_tv]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // ott or token dialog
        if ($this->plugin->config->get_embedded_account() !== null) {
            Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CLEAR_ACCOUNT,
                'Данные для просмотра:', 'Удалить встроенные данные', $setting_icon);
        } else {
            switch ($this->plugin->config->get_feature(Plugin_Constants::ACCESS_TYPE)) {
                case Plugin_Constants::ACCOUNT_OTT_KEY:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_OTTKEY_DLG,
                        'Данные для просмотра:', 'Ввести ОТТ ключ и домен', $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_LOGIN:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_LOGIN_DLG,
                        'Данные для просмотра:', 'Введите логин и пароль', $text_icon);
                    break;
                case Plugin_Constants::ACCOUNT_PIN:
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_PIN_DLG,
                        'Данные для просмотра:', 'Введите ключ доступа', $text_icon);
                    break;
            }
        }

        //////////////////////////////////////
        // channels list source
        $source_ops[1] = 'Локальная или сетевая папки';
        $source_ops[2] = 'Интернет/Интранет папка';
        $source_ops[3] = 'Прямая Интернет ссылка';
        $channels_source = isset($plugin_cookies->channels_source) ? (int)$plugin_cookies->channels_source : 1;

        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANNELS_SOURCE,
            'Источник списка каналов:', $channels_source, $source_ops, self::CONTROLS_WIDTH, true);

        switch ($channels_source)
        {
            case 1: // channels path
                $display_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
                if (strlen($display_path) > 30) {
                    $display_path = "..." . substr($display_path, -30);
                }
                if (is_apk())
                    Control_Factory::add_label($defs, 'Папка со списками каналов:', $display_path);
                else
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANGE_LIST_PATH,
                        'Задать папку со списками каналов:', $display_path, $folder_icon);
                break;
            case 2: // internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    'Задать ссылку со списками каналов:', 'Изменить ссылку', $web_icon, self::CONTROLS_WIDTH);
                break;
            case 3: // direct internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    'Задать ссылку на список каналов:', 'Изменить ссылку', $link_icon, self::CONTROLS_WIDTH);
                break;
        }

        //////////////////////////////////////
        // channels lists
        $all_channels = $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
        if (empty($all_channels)) {
            Control_Factory::add_label($defs, 'Используемый список каналов:', 'Нет списка каналов!!!');
        } else {
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANGE_LIST,
                'Используемый список каналов:', $channels_list, $all_channels, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // streaming dialog
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_STREAMING_DLG,
            'Настройки EPG и проигрывания:', 'Изменить настройки', $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // adult channel password
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_PASS_DLG,
            'Пароль для взрослых каналов:', 'Изменить пароль', $text_icon, self::CONTROLS_WIDTH);

        Control_Factory::add_vgap($defs, 20);

        //////////////////////////////////////
        // adult channel password
        Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_SEND_LOG,
            'Отправить лог разработчику:', 'Отправить', $setting_icon, self::CONTROLS_WIDTH);

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
        Control_Factory::add_text_field($defs, $this, null, 'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'ott_key', 'Введите ключ ОТТ:',
            $ott_key, false, true, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'vportal', 'Введите ключ VPORTAL:',
            $vportal, false, false, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_OTTKEY_APPLY, 'ОК', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * channels list url dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_channels_url_control_defs(&$plugin_cookies)
    {
        hd_print("do_get_channels_url_control_defs");
        $defs = array();

        $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
        $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;
        $url_path = '';
        switch ($source) {
            case 2:
                $url_path = $this->plugin->plugin_info['app_channels_url_path'];
                break;
            case 3:
                if (isset($this->plugin->plugin_info['app_direct_links'][$channels_list])) {
                    $url_path = $this->plugin->plugin_info['app_direct_links'][$channels_list];
                }
                break;
            default:
                break;
        }

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, 'channels_url_path', '',
            $url_path, false, false, false, true, 800);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_APPLY, 'ОК', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
        hd_print("do_get_channels_control_defs");
        $defs = array();
        Control_Factory::add_vgap($defs, 20);

        //////////////////////////////////////
        // auto play
        $on_off_ops = array();
        $on_off_ops[SetupControlSwitchDefs::switch_off] = 'Нет';
        $on_off_ops[SetupControlSwitchDefs::switch_on] = 'Да';
        $auto_play = isset($plugin_cookies->auto_play) ? $plugin_cookies->auto_play : SetupControlSwitchDefs::switch_off;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_AUTO_PLAY, 'Автостарт воспроизведения:', $auto_play, $on_off_ops, 0);

        //////////////////////////////////////
        // auto resume
        $auto_resume = isset($plugin_cookies->auto_resume) ? $plugin_cookies->auto_resume : SetupControlSwitchDefs::switch_on;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_AUTO_RESUME, 'Возобновление просмотра:', $auto_resume, $on_off_ops, 0);

        //////////////////////////////////////
        // strip https
        $strip_https = isset($plugin_cookies->strip_https) ? $plugin_cookies->strip_https : SetupControlSwitchDefs::switch_off;
        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_STRIP_HTTPS, 'Заменять https на http:', $strip_https, $on_off_ops, 0);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_button($defs, $this, null, self::SETUP_ACTION_CLEAR_EPG_CACHE, 'Очистить кэш EPG:', 'Очистить', 0);

        //////////////////////////////////////
        // EPG
        $epg_params = $this->plugin->config->get_epg_params(Plugin_Constants::EPG_SECOND);
        if (!empty($epg_params[Epg_Params::EPG_URL])) {
            $epg_source_ops = array();
            $epg_source_ops[SetupControlSwitchDefs::switch_epg1] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg1];
            $epg_source_ops[SetupControlSwitchDefs::switch_epg2] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg2];

            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;

            Control_Factory::add_combobox($defs, $this, null,
                'epg_source', 'Источник EPG:', $epg_source, $epg_source_ops, 0);
        }

        $epg_font_ops = array();
        $epg_font_ops[SetupControlSwitchDefs::switch_small] = self::$on_off_ops[SetupControlSwitchDefs::switch_small];
        $epg_font_ops[SetupControlSwitchDefs::switch_normal] = self::$on_off_ops[SetupControlSwitchDefs::switch_normal];

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;

        Control_Factory::add_combobox($defs, $this, null,
            'epg_font_size', 'Шрифт EPG:', $epg_font_size, $epg_font_ops, 0);

        //////////////////////////////////////
        // select server
        $servers = $this->plugin->config->get_servers($plugin_cookies);
        if (!empty($servers)) {
            hd_print("Change server supported");
            $server_id = $this->plugin->config->get_server_id($plugin_cookies);
            $server_name = $this->plugin->config->get_server_name($plugin_cookies);
            hd_print("Selected server: id: $server_id name: '$server_name'");
            Control_Factory::add_combobox($defs, $this, null, 'server', 'Сервер:', $server_id, $servers, 0);
        }

        //////////////////////////////////////
        // select device number
        $devices = $this->plugin->config->get_devices($plugin_cookies);
        if (!empty($devices)) {
            hd_print("Change device supported");
            $device_id = $this->plugin->config->get_device_id($plugin_cookies);
            $device_name = $this->plugin->config->get_device_name($plugin_cookies);
            hd_print("Selected device: id: $device_id name: '$device_name'");
            Control_Factory::add_combobox($defs, $this, null, 'device', 'Номер устройства:', $device_id, $devices, 0);
        }

        //////////////////////////////////////
        // select quality
        $qualities = $this->plugin->config->get_qualities($plugin_cookies);
        if (!empty($qualities)) {
            hd_print("Change quality supported");
            $quality_id = $this->plugin->config->get_quality_id($plugin_cookies);
            $quality_name = $this->plugin->config->get_quality_name($plugin_cookies);
            hd_print("Selected quality: id: $quality_id name: '$quality_name'");
            Control_Factory::add_combobox($defs, $this, null, 'quality', 'Качество:', $quality_id, $qualities, 0);
        }

        //////////////////////////////////////
        // select profile
        $profiles = $this->plugin->config->get_profiles($plugin_cookies);
        if (!empty($profiles)) {
            hd_print("Change profile supported");
            $profile_id = $this->plugin->config->get_profile_id($plugin_cookies);
            $profile_name = $this->plugin->config->get_profile_name($plugin_cookies);
            hd_print("Selected profile: id: $profile_id name: '$profile_name'");
            Control_Factory::add_combobox($defs, $this, null, 'profile', 'Профиль:', $profile_id, $profiles, 0);
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
            Control_Factory::add_combobox($defs, $this, null, 'stream_format', 'Выбор потока:', $format_id, $format_ops, 0);
        }

        //////////////////////////////////////
        // buffering time
        $show_buf_time_ops = array();
        $show_buf_time_ops[1000] = '1 с (По умолчанию)';
        $show_buf_time_ops[0] = 'Без буферизации';
        $show_buf_time_ops[500] = '0.5 с';
        $show_buf_time_ops[2000] = '2 с';
        $show_buf_time_ops[3000] = '3 с';
        $show_buf_time_ops[5000] = '5 с';
        $show_buf_time_ops[10000] = '10 с';

        $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
        Control_Factory::add_combobox($defs, $this, null, 'buf_time', 'Время буферизации:', $buf_time, $show_buf_time_ops, 0);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_STREAMING_APPLY, 'ОК', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
                'epg_source', 'Источник EPG:', $epg_source, $epg_source_ops, 0);
        }

        $epg_font_ops = array();
        $epg_font_ops[SetupControlSwitchDefs::switch_small] = self::$on_off_ops[SetupControlSwitchDefs::switch_small];
        $epg_font_ops[SetupControlSwitchDefs::switch_normal] = self::$on_off_ops[SetupControlSwitchDefs::switch_normal];

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;

        Control_Factory::add_combobox($defs, $this, null,
            'epg_font_size', 'Шрифт EPG:', $epg_font_size, $epg_font_ops, 0);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_button($defs, $this, null, self::SETUP_ACTION_CLEAR_EPG_CACHE, 'Очистить кэш EPG:', 'Очистить', 0);

        Control_Factory::add_vgap($defs, 50);
        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_EPG_APPLY, 'Применить', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
        Control_Factory::add_text_field($defs, $this, null, 'login', 'Логин:',
            $login, false, false, false, true, 600);

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
        Control_Factory::add_text_field($defs, $this, null, 'password', 'Пароль:',
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_LOGIN_APPLY, 'Применить', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
        Control_Factory::add_text_field($defs, $this, null, 'password', 'Ключ доступа:',
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_PIN_APPLY, 'Применить', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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

        Control_Factory::add_text_field($defs, $this, null, 'pass1', 'Старый пароль:',
            $pass1, 1, true, 0, 1, 500, 0);
        Control_Factory::add_text_field($defs, $this, null, 'pass2', 'Новый пароль:',
            $pass2, 1, true, 0, 1, 500, 0);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_PASS_APPLY, 'ОК', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
        //hd_print('Setup: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (isset($user_input->action_type) && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $control_id = $user_input->control_id;
            $new_value = '';
            if (isset($user_input->{$control_id})) {
                $new_value = $user_input->{$control_id};
                hd_print("Setup: changing $control_id value to $new_value");
            }

            switch ($control_id) {

                case self::SETUP_ACTION_SHOW_TV:
                    $plugin_cookies->show_tv = ($plugin_cookies->show_tv === SetupControlSwitchDefs::switch_on)
                        ? SetupControlSwitchDefs::switch_off
                        : SetupControlSwitchDefs::switch_on;
                    break;

                case self::SETUP_ACTION_OTTKEY_DLG: // show ott key dialog
                    $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Ключ чувствителен к регистру. Переключение регистра кнопкой Select', $defs, true);

                case self::SETUP_ACTION_OTTKEY_APPLY: // handle ott key dialog result
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    $plugin_cookies->mediateka = $user_input->vportal;
                    hd_print("portal info: $plugin_cookies->mediateka");
                    return $this->reload_channels($plugin_cookies);

                case self::SETUP_ACTION_LOGIN_DLG: // token dialog
                    $defs = $this->do_get_login_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select', $defs, true);

                case self::SETUP_ACTION_LOGIN_APPLY: // handle token dialog result
                    $old_login = isset($plugin_cookies->login) ? $plugin_cookies->login : '';
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->login = $user_input->login;
                    $plugin_cookies->password = $user_input->password;
                    $account_data = $this->plugin->config->GetAccountInfo($plugin_cookies, true);
                    if ($account_data === false) {
                        $plugin_cookies->login = $old_login;
                        $plugin_cookies->password = $old_password;
                        return Action_Factory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels($plugin_cookies);

                case self::SETUP_ACTION_PIN_DLG: // token dialog
                    $defs = $this->do_get_pin_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case self::SETUP_ACTION_PIN_APPLY: // handle token dialog result
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->password = $user_input->password;
                    $account_data = $this->plugin->config->GetAccountInfo($plugin_cookies, true);
                    if ($account_data === false) {
                        $plugin_cookies->password = $old_password;
                        return Action_Factory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels($plugin_cookies);

                case self::SETUP_ACTION_CLEAR_ACCOUNT: // confirm clear account
                    if ($this->plugin->config->get_embedded_account() === null) break;

                    return Action_Factory::show_confirmation_dialog('Внимание!', $this, self::SETUP_ACTION_CLEAR_ACCOUNT_APPLY,
                        'Вы действительно хотите удалить встроенные данные?');

                case self::SETUP_ACTION_CLEAR_ACCOUNT_APPLY: // handle clear account
                    exec('rm -rf ' . get_install_path('account.dat'));
                    exec('rm -rf ' . get_data_path('account.dat'));
                    $this->plugin->config->set_embedded_account(null);
                    $post_action = User_Input_Handler_Registry::create_action($this, 'reset_controls');
                    return Action_Factory::show_title_dialog('Данные удалены', $post_action);

                case self::SETUP_ACTION_CHANGE_LIST_PATH:
                    $media_url = MediaURL::encode(
                        array(
                            'screen_id' => Starnet_Folder_Screen::ID,
                            'save_data' => 'channels_list_path',
                            'windowCounter' => 1,
                        )
                    );
                    return Action_Factory::open_folder($media_url, 'Папка со списком каналов');

                case self::SETUP_ACTION_CHANGE_LIST:
                    $old_value = $plugin_cookies->channels_list;
                    $plugin_cookies->channels_list = $new_value;
                    $action = $this->reload_channels($plugin_cookies);
                    if ($action === null) {
                        $plugin_cookies->channels_list = $old_value;
                        Action_Factory::show_title_dialog("Ошибка загрузки плейлиста!");
                    }
                    return $action;

                case self::SETUP_ACTION_CHANNELS_SOURCE: // handle streaming settings dialog result
                    $plugin_cookies->channels_source = $user_input->channels_source;
                    hd_print("Selected channels source: $plugin_cookies->channels_source");
                    $action = $this->reload_channels($plugin_cookies);
                    if ($action === null) {
                        Action_Factory::show_title_dialog("Ошибка загрузки плейлиста!");
                    }
                    return $action;

                case self::SETUP_ACTION_CHANNELS_URL_DLG:
                    $defs = $this->do_get_channels_url_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Ссылка на списки каналов', $defs, true);

                case self::SETUP_ACTION_CHANNELS_URL_APPLY: // handle streaming settings dialog result
                    $need_reload = false;
                    if (isset($user_input->channels_url_path)) {
                        $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;

                        switch ($source) {
                            case 2:
                                if ($user_input->channels_url_path !== $plugin_cookies->channels_url) {
                                    $plugin_cookies->channels_url = $user_input->channels_url_path;
                                    $need_reload = true;
                                }
                                break;
                            case 3:
                                if ($user_input->channels_url_path !== $plugin_cookies->channels_direct_url) {
                                    $plugin_cookies->channels_direct_url = $user_input->channels_url_path;
                                    $need_reload = true;
                                }
                                break;
                        }

                        hd_print("Selected channels path: $plugin_cookies->channels_url");
                    }

                    return $need_reload ? $this->reload_channels($plugin_cookies) : null;

                case self::SETUP_ACTION_STREAMING_DLG: // show streaming settings dialog
                    $defs = $this->do_get_streaming_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Настройки воспроизведения', $defs, true);

                case self::SETUP_ACTION_STREAMING_APPLY: // handle streaming settings dialog result
                    $plugin_cookies->buf_time = $user_input->buf_time;
                    hd_print("Buffering time: $plugin_cookies->buf_time");

                    $plugin_cookies->auto_play = $user_input->auto_play;
                    hd_print("Auto play: $plugin_cookies->auto_play");

                    $plugin_cookies->auto_resume = $user_input->auto_resume;
                    hd_print("Auto resume: $plugin_cookies->auto_resume");

                    $plugin_cookies->strip_https = $user_input->strip_https;
                    hd_print("Strip https: $plugin_cookies->strip_https");

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

                    return $need_reload ? $this->reload_channels($plugin_cookies) : null;

                case self::SETUP_ACTION_CLEAR_EPG_CACHE: // clear epg cache
                    $epg_path = get_temp_path("epg/");
                    hd_print("do clear epg: $epg_path");
                    foreach(glob($epg_path . "*") as $file) {
                        if(is_file($file)) {
                            hd_print("erase: $file");
                            unlink($file);
                        }
                    }
                    return Action_Factory::show_title_dialog('Кэш EPG очищен');

                case self::SETUP_ACTION_PASS_DLG: // show pass dialog
                    $defs = $this->do_get_pass_control_defs();
                    return Action_Factory::show_dialog('Родительский контроль', $defs, true);

                case self::SETUP_ACTION_PASS_APPLY: // handle pass dialog result
                    if (empty($user_input->pass1) || empty($user_input->pass2)) {
                        return null;
                    }

                    $msg = 'Пароль не изменен!';
                    $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
                    if ($user_input->pass1 === $pass_sex) {
                        $plugin_cookies->pass_sex = $user_input->{'pass2'};
                        $msg = 'Пароль изменен!';
                    }
                    return Action_Factory::show_title_dialog($msg);

                case self::SETUP_ACTION_SEND_LOG: // send log to developer
                    $error_msg = '';
                    $msg = HD::send_log_to_developer($error_msg) ? "Лог отправлен!" : "Лог не отправлен! $error_msg";
                    return Action_Factory::show_title_dialog($msg);
            }
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }

    /**
     * @return array
     */
    protected function reload_channels(&$plugin_cookies)
    {
        hd_print("reload_channels");
        $this->plugin->config->ClearPlaylistCache();
        $this->plugin->config->ClearChannelsCache($plugin_cookies);
        $this->plugin->tv->unload_channels();
        try {
            $this->plugin->tv->load_channels($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Reload channel list failed: $plugin_cookies->channels_list");
            return null;
        }

        $post_action = User_Input_Handler_Registry::create_action($this, 'reset_controls');

        Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
        $post_action = Starnet_Epfs_Handler::invalidate_folders(null, $post_action);

        return Action_Factory::invalidate_folders(array(Starnet_Tv_Groups_Screen::ID), $post_action);
    }
}

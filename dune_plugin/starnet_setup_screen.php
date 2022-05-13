<?php
require_once 'lib/screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'setup';
    const CONTROLS_WIDTH = 800;

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_on => 'Да',
        SetupControlSwitchDefs::switch_off => 'Нет',
        SetupControlSwitchDefs::switch_small => 'Мелкий',
        SetupControlSwitchDefs::switch_normal => 'Обычный',
        SetupControlSwitchDefs::switch_epg1 => 'Нет',
        SetupControlSwitchDefs::switch_epg2 => 'Да',
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
        $title = $this->plugin->config->PLUGIN_SHOW_NAME . ' v.' . $this->plugin->config->PLUGIN_VERSION . ' [' . $this->plugin->config->PLUGIN_DATE . ']';
        Control_Factory::add_label($defs, $title, 'IPTV Channel Editor by sharky72');

        //////////////////////////////////////
        // Show in main screen
        $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        Control_Factory::add_image_button($defs, $this, null, 'show_tv', 'Показывать в главном меню:',
            self::$on_off_ops[$show_tv], $this->plugin->get_image_path(self::$on_off_img[$show_tv]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // ott or token dialog
        $text_icon = $this->plugin->get_image_path('text.png');
        switch ($this->plugin->config->get_feature(ACCOUNT_TYPE)) {
            case 'OTT_KEY':
                if ($this->plugin->config->get_embedded_account() === null) {
                    Control_Factory::add_image_button($defs, $this, null, 'ott_key_dialog',
                        'Активировать просмотр:', 'Ввести ОТТ ключ и домен', $text_icon);
                } else {
                    Control_Factory::add_label($defs, 'Активировать просмотр:', 'Используются встроенные данные');
                }
                break;
            case 'LOGIN':
                if ($this->plugin->config->get_embedded_account() === null) {
                    Control_Factory::add_image_button($defs, $this, null, 'login_dialog',
                        'Активировать просмотр:', 'Введите логин и пароль', $text_icon);
                } else {
                    Control_Factory::add_label($defs, 'Активировать просмотр:', 'Используются встроенные данные');
                }
                break;
            case 'PIN':
                if ($this->plugin->config->get_embedded_account() === null) {
                    Control_Factory::add_image_button($defs, $this, null, 'pin_dialog',
                        'Активировать просмотр:', 'Введите ключ доступа', $text_icon);
                } else {
                    Control_Factory::add_label($defs, 'Активировать просмотр:', 'Используются встроенные данные');
                }
                break;
        }

        //////////////////////////////////////
        // channels path
        $display_path = $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
        if (strlen($display_path) > 30) {
            $display_path = "..." . substr($display_path, -30);
        }
        Control_Factory::add_image_button($defs, $this, null, 'change_list_path',
            'Выбрать папку со списками каналов:', $display_path, $this->plugin->get_image_path('folder.png'));

        //////////////////////////////////////
        // channels lists
        $all_channels = array();
        $list = glob($channels_list_path . '/*.xml');
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename !== 'dune_plugin.xml') {
                $all_channels[$filename] = $filename;
            }
        }
        if (!empty($all_channels)) {
            $channels_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : $this->plugin->config->get_channel_list();
            Control_Factory::add_combobox($defs, $this, null, 'channels_list',
                'Используемый список каналов:', $channels_list, $all_channels, 0, true);
        } else {
            Control_Factory::add_label($defs, 'Используемый список каналов:', 'Нет списка каналов!!!');
        }

        //////////////////////////////////////
        // streaming dialog
        Control_Factory::add_image_button($defs, $this, null, 'streaming_dialog', 'Настройки проигрывания:', 'Изменить настройки',
            $this->plugin->get_image_path('settings.png'));

        //////////////////////////////////////
        // font size
        if (isset($plugin_cookies->has_secondary_epg) && (int)$plugin_cookies->has_secondary_epg === 1) {
            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : SetupControlSwitchDefs::switch_epg1;
            Control_Factory::add_image_button($defs, $this, null, 'epg_source', 'Использовать вторичный источник EPG:',
                self::$on_off_ops[$epg_source], $this->plugin->get_image_path(self::$on_off_img[$epg_source]));
        }

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : SetupControlSwitchDefs::switch_normal;
        Control_Factory::add_image_button($defs, $this, null, 'epg_font_size', 'Мелкий шрифт EPG:',
            self::$on_off_ops[$epg_font_size], $this->plugin->get_image_path(self::$on_off_img[$epg_font_size]));

        //////////////////////////////////////
        // adult channel password
        Control_Factory::add_image_button($defs, $this, null, 'pass_dialog',
            'Пароль для взрослых каналов:', 'Изменить пароль', $text_icon);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_button($defs, $this, null, 'clear_epg_cache', 'Очистить кэш EPG:', 'Очистить', 0);

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

        Control_Factory::add_text_field($defs, $this, null, 'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'ott_key', 'Введите ключ ОТТ:',
            $ott_key, false, true, false, true, 600);

        Control_Factory::add_text_field($defs, $this, null, 'vportal', 'Введите ключ VPORTAL:',
            $vportal, false, false, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, 'ott_key_apply', 'ОК', 300);
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
        hd_print("do_get_streaming_control_defs");
        $defs = array();
        //////////////////////////////////////
        // select device number
        $device_ops = $this->plugin->config->get_feature(DEVICE_OPTIONS);
        if (!empty($device_ops)) {
            hd_print("Change device supported");
            $dev_num = $this->plugin->config->get_device($plugin_cookies);
            Control_Factory::add_combobox($defs, $this, null, 'device', 'Номер устройства:', $dev_num, $device_ops, 0);
        }

        //////////////////////////////////////
        // select server
        if ($this->plugin->config->get_feature(SERVER_SUPPORTED)) {
            hd_print("Change server supported");
            $server_ops = $this->plugin->config->get_server_opts($plugin_cookies);
            $server = $this->plugin->config->get_server($plugin_cookies);
            hd_print("Selected server $server");
            if (!empty($server_ops)) {
                Control_Factory::add_combobox($defs, $this, null, 'server', 'Сервер:', $server, $server_ops, 0);
            }
        }

        //////////////////////////////////////
        // select quality
        if ($this->plugin->config->get_feature(QUALITY_SUPPORTED)) {
            hd_print("Change quality supported");
            $quality = $this->plugin->config->get_quality($plugin_cookies);
            $quality_ops = $this->plugin->config->get_quality_opts($plugin_cookies);
            if (!empty($quality_ops)) {
                Control_Factory::add_combobox($defs, $this, null, 'quality', 'Качество:', $quality, $quality_ops, 0);
            }
        }

        //////////////////////////////////////
        // select stream type
        $format_ops = $this->plugin->config->get_feature(TS_OPTIONS);
        if (count($format_ops) > 1) {
            $format = $this->plugin->config->get_format($plugin_cookies);
            Control_Factory::add_combobox($defs, $this, null, 'stream_format', 'Выбор потока:', $format, $format_ops, 0);
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

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, 'streaming_apply', 'ОК', 300);
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
        Control_Factory::add_text_field($defs, $this, null, 'login', 'Логин:',
            $login, false, false, false, true, 600);

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
        Control_Factory::add_text_field($defs, $this, null, 'password', 'Пароль:',
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, 'login_apply', 'Применить', 300);
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
        Control_Factory::add_text_field($defs, $this, null, 'password', 'Ключ доступа:',
            $password, false, true, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, 'pin_apply', 'Применить', 300);
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

        Control_Factory::add_text_field($defs, $this, null, 'pass1', 'Старый пароль:',
            $pass1, 1, true, 0, 1, 500, 0);
        Control_Factory::add_text_field($defs, $this, null, 'pass2', 'Новый пароль:',
            $pass2, 1, true, 0, 1, 500, 0);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, 'pass_apply', 'ОК', 300);
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
        //foreach ($user_input as $key => $value) {
        //    hd_print("$key => $value");
        //}

        if (isset($user_input->action_type) && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $control_id = $user_input->control_id;
            $new_value = '';
            if (isset($user_input->{$control_id})) {
                $new_value = $user_input->{$control_id};
                hd_print("Setup: changing $control_id value to $new_value");
            }

            switch ($control_id) {

                case 'show_tv':
                    $plugin_cookies->show_tv = ($plugin_cookies->show_tv === SetupControlSwitchDefs::switch_on)
                        ? SetupControlSwitchDefs::switch_off
                        : SetupControlSwitchDefs::switch_on;
                    break;

                case 'ott_key_dialog': // show ott key dialog
                    $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Ключ чувствителен к регистру. Переключение регистра кнопкой Select', $defs, true);

                case 'ott_key_apply': // handle ott key dialog result
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    $plugin_cookies->mediateka = $user_input->vportal;
                    hd_print("portal info: $plugin_cookies->mediateka");
                    return $this->reload_channels($plugin_cookies);

                case 'login_dialog': // token dialog
                    $defs = $this->do_get_login_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select', $defs, true);

                case 'login_apply': // handle token dialog result
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

                case 'pin_dialog': // token dialog
                    $defs = $this->do_get_pin_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case 'pin_apply': // handle token dialog result
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->password = $user_input->password;
                    $account_data = $this->plugin->config->GetAccountInfo($plugin_cookies, true);
                    if ($account_data === false) {
                        $plugin_cookies->password = $old_password;
                        return Action_Factory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels($plugin_cookies);

                case 'change_list_path':
                    $media_url = MediaURL::encode(
                        array(
                            'screen_id' => Starnet_Folder_Screen::ID,
                            'save_data' => 'ch_list_path',
                            'windowCounter' => 1,
                        )
                    );
                    return Action_Factory::open_folder($media_url, 'Папка со списком каналов');

                case 'channels_list':
                    $old_value = $plugin_cookies->channels_list;
                    $plugin_cookies->channels_list = $new_value;
                    $res = $this->reload_channels($plugin_cookies);
                    if ($res === false) {
                        $plugin_cookies->channels_list = $old_value;
                        Action_Factory::show_title_dialog("Ошибка загрузки плейлиста!");
                    }
                    $post_action = User_Input_Handler_Registry::create_action($this, 'reset_controls');
                    return Action_Factory::invalidate_folders(array('tv_group_list'), $post_action);

                case 'epg_source':
                    if (isset($plugin_cookies->epg_source)) {
                        $plugin_cookies->epg_source = ($plugin_cookies->epg_source === SetupControlSwitchDefs::switch_epg1)
                            ? SetupControlSwitchDefs::switch_epg2
                            : SetupControlSwitchDefs::switch_epg1;
                    } else {
                        $plugin_cookies->epg_source = SetupControlSwitchDefs::switch_epg1;
                    }
                    break;

                case 'epg_font_size':
                    if (isset($plugin_cookies->epg_font_size)) {
                        $plugin_cookies->epg_font_size = ($plugin_cookies->epg_font_size === SetupControlSwitchDefs::switch_normal)
                            ? SetupControlSwitchDefs::switch_small
                            : SetupControlSwitchDefs::switch_normal;
                    } else {
                        $plugin_cookies->epg_font_size = SetupControlSwitchDefs::switch_small;
                    }
                    break;

                case 'streaming_dialog': // show streaming settings dialog
                    $defs = $this->do_get_streaming_control_defs($plugin_cookies);
                    return Action_Factory::show_dialog('Настройки воспроизведения', $defs, true);

                case 'streaming_apply': // handle streaming settings dialog result
                    $plugin_cookies->buf_time = $user_input->buf_time;

                    if (isset($user_input->stream_format)) {
                        $plugin_cookies->format = $user_input->stream_format;
                    }

                    if (isset($user_input->device) && $plugin_cookies->device_number !== $user_input->device) {
                        $this->plugin->config->set_device($user_input->device, $plugin_cookies);
                    }

                    if (isset($user_input->server) && $plugin_cookies->server !== $user_input->server) {
                        $this->plugin->config->set_server($user_input->server, $plugin_cookies);
                    }

                    if (isset($user_input->quality) && $plugin_cookies->quality !== $user_input->quality) {
                        $plugin_cookies->quality = $user_input->quality;
                        $this->plugin->config->set_quality($plugin_cookies);
                    }
                    return $this->reload_channels($plugin_cookies);

                case 'pass_dialog': // show pass dialog
                    $defs = $this->do_get_pass_control_defs();
                    return Action_Factory::show_dialog('Родительский контроль', $defs, true);

                case 'pass_apply': // handle pass dialog result
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
                case 'clear_epg_cache': // clear epg cache
                    $epg_path = DuneSystem::$properties['tmp_dir_path'] . "/epg";
                    hd_print("do clear epg: $epg_path");
                    foreach(glob($epg_path . "/*") as $file) {
                        if(is_file($file)) {
                            hd_print("erase: $file");
                            unlink($file);
                        }
                    }
                    return Action_Factory::show_title_dialog('Кэш EPG очищен');
            }
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }

    /**
     * @return array|false
     */
    protected function reload_channels(&$plugin_cookies)
    {
        hd_print("reload_channels");
        $this->plugin->tv->unload_channels();
        try {
            $this->plugin->tv->load_channels($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Reload channel list failed: $plugin_cookies->channels_list");
            return false;
        }
        $post_action = User_Input_Handler_Registry::create_action($this, 'reset_controls');
        return Action_Factory::invalidate_folders(array('tv_group_list'), $post_action);
    }
}

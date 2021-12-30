<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class StarnetSetupScreen extends AbstractControlsScreen
{
    const ID = 'setup';
    const EPG_FONTSIZE_DEF_VALUE = 'normal';
    public static $config;

    ///////////////////////////////////////////////////////////////////////
    protected $tv;

    public function __construct(Tv $tv)
    {
        parent::__construct(self::ID);

        $this->tv = $tv;
    }

    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

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
        ControlFactory::add_vgap($defs, -10);
        $config = self::$config;
        $title = $config::$PLUGIN_SHOW_NAME . ' v.' . $config::$PLUGIN_VERSION . ' [' . $config::$PLUGIN_DATE . ']';
        $this->add_label($defs, $title, 'IPTV Channel Editor by sharky72');

        //////////////////////////////////////
        // Show in main screen
        $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        $show_ops = array('yes' => 'Да', 'no' => 'Нет');
        $this->add_combobox($defs, 'show_tv', 'Показывать в главном меню:', $show_tv, $show_ops, 0, true);

        //////////////////////////////////////
        // ott or token dialog
        switch (self::$config->get_account_type())
        {
            case 'OTT_KEY':
                $this->add_button($defs, 'ott_key_dialog', 'Активировать просмотр:', 'Ввести ОТТ ключ и домен', 0);
                break;
            case 'LOGIN':
                $this->add_button($defs, 'login_dialog', 'Активировать просмотр:', 'Введите логин и пароль', 0);
                break;
            case 'PIN':
                $this->add_button($defs, 'pin_dialog', 'Активировать просмотр:', 'Введите ключ доступа', 0);
                break;
        }

        //////////////////////////////////////
        // channels path
        $display_path = $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
        if (strlen($display_path) > 30) {
            $display_path = "..." . substr($display_path, -30);
        }
        $this->add_button($defs, 'change_list_path', 'Выбрать папку со списками каналов:', $display_path, 800);
        $this->add_button($defs, 'reset_path', 'Установить папку по умолчанию:', 'Установить', 800);

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
            $channels_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : self::$config->get_channel_list();
            $this->add_combobox($defs, 'channels_list', 'Используемый список каналов:', $channels_list, $all_channels, 0, true);
        } else {
            $this->add_label($defs, 'Используемый список каналов:', 'Нет списка каналов!!!');
        }

        //////////////////////////////////////
        // streaming dialog
        $this->add_button($defs, 'streaming_dialog', 'Настройки проигрывания:', 'Изменить настройки', 0);

        //////////////////////////////////////
        // font size
        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : self::EPG_FONTSIZE_DEF_VALUE;
        $epg_font_size_ops = array('normal' => 'Обычный', 'small' => 'Мелкий');
        $this->add_combobox($defs, 'epg_font_size', 'Размер шрифта EPG:', $epg_font_size, $epg_font_size_ops, 700, true);

        //////////////////////////////////////
        // adult channel password
        $this->add_button($defs, 'pass_dialog', 'Пароль для взрослых каналов:', 'Изменить пароль', 0);
        ControlFactory::add_vgap($defs, 10);

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

        $this->add_text_field($defs, 'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 600);

        $this->add_text_field($defs, 'ott_key', 'Введите ОТТ ключ:',
            $ott_key, false, true, false, true, 600);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'ott_key_apply', 'ОК', 300);
        $this->add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * streaming parameters dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_streaming_control_defs(&$plugin_cookies)
    {
        $defs = array();
        //////////////////////////////////////
        // select device number
        if (self::$config->get_device_support()) {
            $dev_num = isset($plugin_cookies->device_number) ? $plugin_cookies->device_number : '1';
            $device_ops = array('1' => '1', '2' => '2', '3' => '3');
            $this->add_combobox($defs, 'devices', 'Номер устройства:', $dev_num, $device_ops, 0);
        }

        //////////////////////////////////////
        // select stream type
        $format_ops = array('hls' => 'HLS');
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        if (self::$config->get_hls2_support()) {
            $format_ops['hls2'] = 'HLS2';
        }

        if (self::$config->get_mpeg_support()) {
            $format_ops['mpeg'] = 'MPEG-TS';
        }

        if (count($format_ops) > 1) {
            $this->add_combobox($defs, 'stream_format', 'Выбор потока:', $format, $format_ops, 0);
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
        $this->add_combobox($defs, 'buf_time', 'Время буферизации:', $buf_time, $show_buf_time_ops, 0);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'streaming_apply', 'ОК', 300);
        $this->add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

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
        $this->add_text_field($defs, 'login', 'Логин:',
            $login, false, false, false, true, 600);

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
        $this->add_text_field($defs, 'password', 'Пароль:',
            $password, false, true, false, true, 600);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'login_apply', 'Применить', 300);
        $this->add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

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
        $this->add_text_field($defs, 'password', 'Ключ доступа:',
            $password, false, true, false, true, 600);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'pin_apply', 'Применить', 300);
        $this->add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

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

        $this->add_text_field($defs, 'pass1', 'Старый пароль:',
            $pass1, 1, true, 0, 1, 500, 0);
        $this->add_text_field($defs, 'pass2', 'Новый пароль:',
            $pass2, 1, true, 0, 1, 500, 0);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'pass_apply', 'ОК', 300);
        $this->add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * user remote input handler
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
                case 'change_list_path':
                    $media_url = MediaURL::encode(
                        array(
                            'screen_id' => 'file_list',
                            'save_data' => 'ch_list_path',
                        )
                    );

                    return ActionFactory::open_folder($media_url,'Папка со списком каналов');

                case 'reset_path':
                    hd_print("reset path to default");
                    $plugin_cookies->ch_list_path = '';
                    return $this->reload_channels();

                case 'channels_list':
                    $old_value = $plugin_cookies->channels_list;
                    $this->tv->unload_channels();
                    try {
                        $plugin_cookies->channels_list = $new_value;
                        $this->tv->load_channels($plugin_cookies);
                    } catch (Exception $e) {
                        hd_print("Load channel list failed: $new_value");
                        $plugin_cookies->channels_list = $old_value;
                        ActionFactory::show_title_dialog("Ошибка загрузки плейлиста! " . $e->getMessage());
                    }
                    $post_action = UserInputHandlerRegistry::create_action($this, 'reset_controls');
                    return ActionFactory::invalidate_folders(array('tv_group_list'), $post_action);

                case 'show_tv':
                    $plugin_cookies->show_tv = $new_value;
                    break;

                case 'epg_font_size':
                    $plugin_cookies->epg_font_size = $new_value;
                    break;

                case 'streaming_dialog': // show streaming settings dialog
                    $defs = $this->do_get_streaming_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Настройки воспроизведения', $defs, true);

                case 'streaming_apply': // handle streaming settings dialog result
                    $plugin_cookies->buf_time = $user_input->buf_time;
                    $plugin_cookies->format = $user_input->stream_format;
                    $plugin_cookies->device_number = $user_input->devices;
                    return $this->reload_channels();

                case 'ott_key_dialog': // show ott key dialog
                    $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Ключ чувствителен к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case 'ott_key_apply': // handle ott key dialog result
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    return $this->reload_channels();

                case 'login_dialog': // token dialog
                    $defs = $this->do_get_login_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case 'login_apply': // handle token dialog result
                    $old_login = isset($plugin_cookies->login) ? $plugin_cookies->login : '';
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->login = $user_input->login;
                    $plugin_cookies->password = $user_input->password;
                    $account_data = array();
                    if (!self::$config->GetAccountInfo($plugin_cookies, $account_data, true)) {
                        $plugin_cookies->login = $old_login;
                        $plugin_cookies->password = $old_password;
                        return ActionFactory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels();

                case 'pin_dialog': // token dialog
                    $defs = $this->do_get_pin_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case 'pin_apply': // handle token dialog result
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->password = $user_input->password;
                    $account_data = array();
                    if (!self::$config->GetAccountInfo($plugin_cookies, $account_data, true)) {
                        $plugin_cookies->password = $old_password;
                        return ActionFactory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels();

                case 'pass_dialog': // show pass dialog
                    $defs = $this->do_get_pass_control_defs();
                    return ActionFactory::show_dialog('Родительский контроль', $defs, true);

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
                    return ActionFactory::show_title_dialog($msg);
            }
        }

        return ActionFactory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }

    /**
     * @return array
     */
    protected function reload_channels()
    {
        $this->tv->unload_channels();
        try {
            $this->tv->load_channels($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Reload channel list failed: $plugin_cookies->channels_list");
        }
        $post_action = UserInputHandlerRegistry::create_action($this, 'reset_controls');
        return ActionFactory::invalidate_folders(array('tv_group_list'), $post_action);
    }
}

<?php
require_once 'lib/screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class ControlSwitchDefs
{
    const switch_on  = 'yes';
    const switch_off = 'no';
    const switch_normal  = 'normal';
    const switch_small = 'small';
    const switch_epg1  = 'first';
    const switch_epg2 = 'second';
}

class StarnetSetupScreen extends AbstractControlsScreen implements UserInputHandler
{
    const ID = 'setup';
    const CONTROLS_WIDTH = 800;

    private static $on_off_ops = array
    (
        ControlSwitchDefs::switch_on => 'Да',
        ControlSwitchDefs::switch_off => 'Нет',
        ControlSwitchDefs::switch_small => 'Мелкий',
        ControlSwitchDefs::switch_normal => 'Обычный',
        ControlSwitchDefs::switch_epg1 => 'Нет',
        ControlSwitchDefs::switch_epg2 => 'Да',
    );

    private static $on_off_img = array
    (
        ControlSwitchDefs::switch_on => 'on.png',
        ControlSwitchDefs::switch_off => 'off.png',
        ControlSwitchDefs::switch_small => 'on.png',
        ControlSwitchDefs::switch_normal => 'off.png',
        ControlSwitchDefs::switch_epg1 => 'off.png',
        ControlSwitchDefs::switch_epg2 => 'on.png',
    );

    ///////////////////////////////////////////////////////////////////////

    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin);

        $plugin->create_screen($this);
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
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
        ControlFactory::add_vgap($defs, -10);
        $title = $this->plugin->config->PLUGIN_SHOW_NAME . ' v.' . $this->plugin->config->PLUGIN_VERSION . ' [' . $this->plugin->config->PLUGIN_DATE . ']';
        ControlFactory::add_label($defs, $title, 'IPTV Channel Editor by sharky72');

        //////////////////////////////////////
        // Show in main screen
        $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        ControlFactory::add_image_button($defs, $this, null, 'show_tv', 'Показывать в главном меню:',
            self::$on_off_ops[$show_tv], $this->plugin->get_image_path(self::$on_off_img[$show_tv]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // ott or token dialog
        switch ($this->plugin->config->get_account_type())
        {
            case 'OTT_KEY':
                ControlFactory::add_image_button($defs, $this, null, 'ott_key_dialog', 'Активировать просмотр:', 'Ввести ОТТ ключ и домен',
                    $this->plugin->get_image_path('text.png'));
                break;
            case 'LOGIN':
                ControlFactory::add_image_button($defs, $this, null, 'login_dialog', 'Активировать просмотр:', 'Введите логин и пароль',
                    $this->plugin->get_image_path('text.png'));
                break;
            case 'PIN':
                ControlFactory::add_image_button($defs, $this, null, 'pin_dialog', 'Активировать просмотр:', 'Введите ключ доступа',
                    $this->plugin->get_image_path('text.png'));
                break;
        }

        //////////////////////////////////////
        // vportal dialog
        if ($this->plugin->config->get_vod_portal_support()) {
            ControlFactory::add_image_button($defs, $this, null, 'portal_dialog', 'Активировать VPortal:', 'Ввести ключ',
                $this->plugin->get_image_path('text.png'));
        }

        //////////////////////////////////////
        // channels path
        $display_path = $channels_list_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
        if (strlen($display_path) > 30) {
            $display_path = "..." . substr($display_path, -30);
        }
        ControlFactory::add_image_button($defs, $this, null, 'change_list_path', 'Выбрать папку со списками каналов:', $display_path,
            $this->plugin->get_image_path('folder.png'));

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
            ControlFactory::add_combobox($defs, $this, null, 'channels_list', 'Используемый список каналов:', $channels_list, $all_channels, 0, true);
        } else {
            ControlFactory::add_label($defs, 'Используемый список каналов:', 'Нет списка каналов!!!');
        }

        //////////////////////////////////////
        // streaming dialog
        ControlFactory::add_image_button($defs, $this, null, 'streaming_dialog', 'Настройки проигрывания:', 'Изменить настройки',
            $this->plugin->get_image_path('settings.png'));

        //////////////////////////////////////
        // font size
        if (isset($plugin_cookies->has_secondary_epg) && (int)$plugin_cookies->has_secondary_epg === 1) {
            $epg_source = isset($plugin_cookies->epg_source) ? $plugin_cookies->epg_source : ControlSwitchDefs::switch_epg1;
            ControlFactory::add_image_button($defs, $this, null, 'epg_source', 'Использовать вторичный источник EPG:',
                self::$on_off_ops[$epg_source], $this->plugin->get_image_path(self::$on_off_img[$epg_source]));
        }

        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : ControlSwitchDefs::switch_normal;
        ControlFactory::add_image_button($defs, $this, null, 'epg_font_size', 'Мелкий шрифт EPG:',
            self::$on_off_ops[$epg_font_size], $this->plugin->get_image_path(self::$on_off_img[$epg_font_size]));

        //////////////////////////////////////
        // adult channel password
        ControlFactory::add_image_button($defs, $this, null, 'pass_dialog', 'Пароль для взрослых каналов:', 'Изменить пароль',
            $this->plugin->get_image_path('text.png'));

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

        ControlFactory::add_text_field($defs, $this, null, 'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 600);

        ControlFactory::add_text_field($defs, $this, null, 'ott_key', 'Введите ОТТ ключ:',
            $ott_key, false, true, false, true, 600);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'ott_key_apply', 'ОК', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
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
        hd_print("do_get_streaming_control_defs");
        $defs = array();
        //////////////////////////////////////
        // select device number
        if ($this->plugin->config->get_device_support()) {
            hd_print("Change device supported");
            $dev_num = $this->plugin->config->get_device($plugin_cookies);
            $device_ops = $this->plugin->config->get_device_opts();
            if (!empty($device_ops)) {
                ControlFactory::add_combobox($defs, $this, null, 'devices', 'Номер устройства:', $dev_num, $device_ops, 0);
            }
        }

        //////////////////////////////////////
        // select server
        if ($this->plugin->config->get_server_support()) {
            hd_print("Change server supported");
            $server = $this->plugin->config->get_server($plugin_cookies);
            $server_ops = $this->plugin->config->get_server_opts($plugin_cookies);
            hd_print("Selected server $server");
            if (!empty($server_ops)) {
                ControlFactory::add_combobox($defs, $this, null, 'server', 'Сервер:', $server, $server_ops, 0);
            }
        }

        //////////////////////////////////////
        // select quality
        if ($this->plugin->config->get_quality_support()) {
            hd_print("Change quality supported");
            $quality = $this->plugin->config->get_quality($plugin_cookies);
            $quality_ops = $this->plugin->config->get_quality_opts($plugin_cookies);
            if (!empty($quality_ops)) {
                ControlFactory::add_combobox($defs, $this, null, 'quality', 'Качество:', $quality, $quality_ops, 0);
            }
        }

        //////////////////////////////////////
        // select stream type
        $format_ops = $this->plugin->config->get_format_opts();
        if (count($format_ops) > 1) {
            $format = $this->plugin->config->get_format($plugin_cookies);
            ControlFactory::add_combobox($defs, $this, null, 'stream_format', 'Выбор потока:', $format, $format_ops, 0);
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
        ControlFactory::add_combobox($defs, $this, null, 'buf_time', 'Время буферизации:', $buf_time, $show_buf_time_ops, 0);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'streaming_apply', 'ОК', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
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
        ControlFactory::add_text_field($defs, $this, null, 'login', 'Логин:',
            $login, false, false, false, true, 600);

        $password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
        ControlFactory::add_text_field($defs, $this, null, 'password', 'Пароль:',
            $password, false, true, false, true, 600);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'login_apply', 'Применить', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
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
        ControlFactory::add_text_field($defs, $this, null, 'password', 'Ключ доступа:',
            $password, false, true, false, true, 600);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'pin_apply', 'Применить', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * portal dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_portal_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $url = isset($plugin_cookies->mediateka) ? $plugin_cookies->mediateka : '';
        ControlFactory::add_text_field($defs, $this, null, 'url', 'Ссылка на VPortal:',
            $url, false, false, false, true, 800);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'portal_apply', 'Применить', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
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

        ControlFactory::add_text_field($defs, $this, null, 'pass1', 'Старый пароль:',
            $pass1, 1, true, 0, 1, 500, 0);
        ControlFactory::add_text_field($defs, $this, null, 'pass2', 'Новый пароль:',
            $pass2, 1, true, 0, 1, 500, 0);

        ControlFactory::add_vgap($defs, 50);

        ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'pass_apply', 'ОК', 300);
        ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
        ControlFactory::add_vgap($defs, 10);

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
                    $plugin_cookies->show_tv = ($plugin_cookies->show_tv === ControlSwitchDefs::switch_on)
                        ? ControlSwitchDefs::switch_off
                        : ControlSwitchDefs::switch_on;
                    break;

                case 'ott_key_dialog': // show ott key dialog
                    $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Ключ чувствителен к регистру. Переключение регистра кнопкой Select', $defs, true);

                case 'ott_key_apply': // handle ott key dialog result
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    return $this->reload_channels($plugin_cookies);

                case 'login_dialog': // token dialog
                    $defs = $this->do_get_login_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select', $defs, true);

                case 'login_apply': // handle token dialog result
                    $old_login = isset($plugin_cookies->login) ? $plugin_cookies->login : '';
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->login = $user_input->login;
                    $plugin_cookies->password = $user_input->password;
                    $account_data = array();
                    if (!$this->plugin->config->GetAccountInfo($plugin_cookies, $account_data, true)) {
                        $plugin_cookies->login = $old_login;
                        $plugin_cookies->password = $old_password;
                        return ActionFactory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels($plugin_cookies);

                case 'pin_dialog': // token dialog
                    $defs = $this->do_get_pin_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);

                case 'pin_apply': // handle token dialog result
                    $old_password = isset($plugin_cookies->password) ? $plugin_cookies->password : '';
                    $plugin_cookies->password = $user_input->password;
                    $account_data = array();
                    if (!$this->plugin->config->GetAccountInfo($plugin_cookies, $account_data, true)) {
                        $plugin_cookies->password = $old_password;
                        return ActionFactory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                    }

                    return $this->reload_channels($plugin_cookies);

                case 'portal_dialog': // portal dialog
                    $defs = $this->do_get_portal_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Переключение регистра кнопкой Select',
                        $defs, true);

                case 'portal_apply': // handle portal dialog result
                    $plugin_cookies->mediateka = $user_input->url;
                    hd_print("portal info: $plugin_cookies->mediateka");
                    return null;

                case 'change_list_path':
                    $media_url = MediaURL::encode(
                        array(
                            'screen_id' => 'file_list',
                            'save_data' => 'ch_list_path',
                            'windowCounter' => 1,
                        )
                    );
                    return ActionFactory::open_folder($media_url,'Папка со списком каналов');

                case 'channels_list':
                    $old_value = $plugin_cookies->channels_list;
                    $this->plugin->tv->unload_channels();
                    try {
                        $plugin_cookies->channels_list = $new_value;
                        $this->plugin->tv->load_channels($plugin_cookies);
                    } catch (Exception $e) {
                        hd_print("Load channel list failed: $new_value");
                        $plugin_cookies->channels_list = $old_value;
                        ActionFactory::show_title_dialog("Ошибка загрузки плейлиста! " . $e->getMessage());
                    }
                    $post_action = UserInputHandlerRegistry::create_action($this, 'reset_controls');
                    return ActionFactory::invalidate_folders(array('tv_group_list'), $post_action);

                case 'epg_source':
                    if (isset($plugin_cookies->epg_source)) {
                        $plugin_cookies->epg_source = ($plugin_cookies->epg_source === ControlSwitchDefs::switch_epg1)
                            ? ControlSwitchDefs::switch_epg2
                            : ControlSwitchDefs::switch_epg1;
                    } else {
                        $plugin_cookies->epg_source = ControlSwitchDefs::switch_epg1;
                    }
                    break;

                case 'epg_font_size':
                    if (isset($plugin_cookies->epg_font_size)) {
                        $plugin_cookies->epg_font_size = ($plugin_cookies->epg_font_size === ControlSwitchDefs::switch_normal)
                            ? ControlSwitchDefs::switch_small
                            : ControlSwitchDefs::switch_normal;
                    } else {
                        $plugin_cookies->epg_font_size = ControlSwitchDefs::switch_normal;
                    }
                    break;

                case 'streaming_dialog': // show streaming settings dialog
                    $defs = $this->do_get_streaming_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Настройки воспроизведения', $defs, true);

                case 'streaming_apply': // handle streaming settings dialog result
                    $plugin_cookies->buf_time = $user_input->buf_time;

                    if (isset($user_input->stream_format)) {
                        $plugin_cookies->format = $user_input->stream_format;
                    }

                    if (isset($user_input->devices) && $plugin_cookies->device_number !== $user_input->devices) {
                        $plugin_cookies->device_number = $user_input->devices;
                        $this->plugin->config->set_device($plugin_cookies);
                    }

                    if (isset($user_input->server) && $plugin_cookies->server !== $user_input->server) {
                        $plugin_cookies->server = $user_input->server;
                        $this->plugin->config->set_server($plugin_cookies);
                    }

                    if (isset($user_input->quality) && $plugin_cookies->quality !== $user_input->quality) {
                        $plugin_cookies->quality = $user_input->quality;
                        $this->plugin->config->set_quality($plugin_cookies);
                    }
                    return $this->reload_channels($plugin_cookies);

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
    protected function reload_channels(&$plugin_cookies)
    {
        hd_print("reload_channels");
        $this->plugin->tv->unload_channels();
        try {
            $this->plugin->tv->load_channels($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Reload channel list failed: $plugin_cookies->channels_list");
        }
        $post_action = UserInputHandlerRegistry::create_action($this, 'reset_controls');
        return ActionFactory::invalidate_folders(array('tv_group_list'), $post_action);
    }
}

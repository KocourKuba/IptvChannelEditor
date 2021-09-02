﻿<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class StarnetSetupScreen extends AbstractControlsScreen
{
    const ID = 'setup';
    const EPG_FONTSIZE_DEF_VALUE = 'normal';

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

    public function do_get_control_defs(&$plugin_cookies)
    {
        $defs = array();
        $config = $this->tv->get_config();

        $format        = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        $channels_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : $config->GET_CHANNEL_LIST_URL();
        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : self::EPG_FONTSIZE_DEF_VALUE;
        $show_tv       = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        $buf_time      = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;

        $show_ops      = array();
        $show_ops['yes'] = 'Да';
        $show_ops['no'] = 'Нет';

        //////////////////////////////////////
        // Plugin name
        ControlFactory::add_vgap($defs, -10);
        $title = $config->GET_PLUGIN_NAME() . ' v.' . $config->GET_PLUGIN_VERSION() . '. [' . $config->GET_PLUGIN_DATE() . ']';
        $this->add_button($defs, 'restart', $title, 'Перезагрузить плеер', 0);

        //////////////////////////////////////
        // Show in main screen
        $this->add_combobox($defs,'show_tv', 'Показывать ' . $config->GET_PLUGIN_NAME() .' в главном меню:',
            $show_tv, $show_ops, 0, true);

        //////////////////////////////////////
        // ott or token dialog
        if ($config->GET_USE_TOKEN()) {
            $this->add_button($defs, 'token_dialog', 'Активировать просмотр:', 'Введите логин и пароль', 0);
        } else {
            $this->add_button($defs, 'ott_key_dialog', 'Активировать просмотр:', 'Ввести ОТТ ключ и домен', 0);
        }

        //////////////////////////////////////
        // channels lists
        $channels = array();
        $list = glob(DuneSystem::$properties['install_dir_path'] . "/*.xml");
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename != 'dune_plugin.xml')
                $channels[$filename] = $filename;
        }

        $this->add_combobox($defs,'channels_list', 'Используемый список каналов:',
            $channels_list, $channels, 0, true);

        //////////////////////////////////////
        // select stream type
        if ($config->GET_MPEG_TS_SUPPORTED()) {
            $format_ops = array();
            $format_ops['hls'] = 'HLS';
            $format_ops['mpeg'] = 'MPEG-TS';
            $this->add_combobox($defs, 'format', 'Выбор потока:', $format, $format_ops, 0, true);
        }

        //////////////////////////////////////
        // buffering time
        $show_buf_time_ops = array();
        $show_buf_time_ops[0] = 'По умолчанию';
        $show_buf_time_ops[500] = '0.5 с';
        $show_buf_time_ops[1000] = '1 с';
        $show_buf_time_ops[2000] = '2 с';
        $show_buf_time_ops[3000] = '3 с';
        $show_buf_time_ops[5000] = '5 с';
        $show_buf_time_ops[10000] = '10 с';

        $this->add_combobox($defs, 'buf_time', 'Время буферизации:',
            $buf_time, $show_buf_time_ops, 0, true);

        //////////////////////////////////////
        // font size
        $epg_font_size_ops = array();
        $epg_font_size_ops ['normal'] = 'Обычный';
        $epg_font_size_ops ['small'] = 'Мелкий';
        $this->add_combobox($defs, 'epg_font_size', 'Размер шрифта EPG:',
            $epg_font_size, $epg_font_size_ops, 700, true);

        //////////////////////////////////////
        // adult channel password
        $this->add_button($defs, 'pass_dialog', 'Пароль для взрослых каналов:', 'Изменить пароль', 0);
        ControlFactory::add_vgap($defs, -10);

        return $defs;
    }

    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs($plugin_cookies);
    }

    /**
     * ott key dialog
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_ott_key_control_defs(&$plugin_cookies)
    {
        $defs = array();
        $ott_key = isset($plugin_cookies->ott_key) ? $plugin_cookies->ott_key : '';

        $this->add_text_field($defs,'ott_key', 'Введите ОТТ ключ:',
            $ott_key, false, false, false, true, 500);

        $subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : '';

        $this->add_text_field($defs,'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 500);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs,'ott_key_apply', 'ОК', 300);
        $this->add_close_dialog_button($defs,'Отмена', 300);

        return $defs;
    }

    /**
     * token dialog
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_token_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $this->add_text_field($defs, 'token', 'Логин:',
            $plugin_cookies->token, false, false, false, true, 500);
        $this->add_text_field($defs, 'pin', 'Пароль:',
            $plugin_cookies->pin, false, false, false, true, 500);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs, 'token_apply', 'Применить', 300);
        $this->add_close_dialog_button($defs,'Отмена', 300);

        return $defs;
    }

    /**
     * adult pass dialog
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_pass_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $pass1 = '';
        $pass2 = '';

        $this->add_text_field($defs,'pass1', 'Старый пароль:',
            $pass1, 1, 1, 0, 1, 500, 0);
        $this->add_text_field($defs,'pass2', 'Новый пароль:',
            $pass2, 1, 1, 0, 1, 500, 0);

        $this->add_vgap($defs, 50);

        $this->add_close_dialog_and_apply_button($defs,'pass_apply', 'ОК', 300);
        $this->add_close_dialog_button($defs,'Отмена', 300);

        return $defs;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Setup: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("$key => $value");

        if ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply') {
            $control_id = $user_input->control_id;
            $new_value = '';
            if (isset($user_input->{$control_id})) {
                $new_value = $user_input->{$control_id};
                hd_print("Setup: changing $control_id value to $new_value");
            }

            switch ($control_id)
            {
                case 'restart':
                    shell_exec('reboot now');
                    break;
                case 'channels_list':
                    $old_value = $plugin_cookies->channels_list;
                    $this->tv->unload_channels();
                    try {
                        $plugin_cookies->channels_list = $new_value;
                        $this->tv->load_channels($plugin_cookies);
                    } catch (Exception $e) {
                        hd_print("Load channel list failed: $new_value");
                        $plugin_cookies->channels_list = $old_value;
                        ActionFactory::show_title_dialog('Ошибка загрузки плейлиста!');
                    }
                    $perform_new_action = UserInputHandlerRegistry::create_action($this, 'reset_controls');
                    return ActionFactory::invalidate_folders(array('tv_group_list'), $perform_new_action);
                case 'show_tv':
                    $plugin_cookies->show_tv = $new_value;
                    break;
                case 'buf_time':
                    $plugin_cookies->buf_time = $new_value;
                    break;
                case 'epg_font_size':
                    $plugin_cookies->epg_font_size = $new_value;
                    break;
                case 'ott_key_dialog': // show ott key dialog
                    $defs = $this->do_get_ott_key_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Ключ чувствителен к регистру. Переключение регистра кнопкой Select',
                        $defs, true);
                case 'ott_key_apply': // handle ott key dialog result
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    break;
                case 'token_dialog': // token dialog
                    $defs = $this->do_get_token_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Данные чувствительны к регистру. Переключение регистра кнопкой Select',
                        $defs, true);
                case 'token_apply': // handle token dialog result
                    $plugin_cookies->token = $user_input->token;
                    $plugin_cookies->pin = $user_input->pin;
                    if ($this->tv->get_config()->GetAccessInfo($plugin_cookies)) break;
                    return ActionFactory::show_title_dialog('Неправильные логин/пароль или неактивна подписка');
                case 'pass_dialog': // show pass dialog
                    $defs = $this->do_get_pass_control_defs($plugin_cookies);
                    return ActionFactory::show_dialog('Родительский контроль', $defs, true);
                case 'pass_apply': // handle pass dialog result
                    if ($user_input->pass1 == '' || $user_input->pass2 == '')
                        return null;

                    $msg = 'Пароль не изменен!';
                    $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
                    if ($user_input->pass1 == $pass_sex) {
                        $plugin_cookies->pass_sex = $user_input->{'pass2'};
                        $msg = 'Пароль изменен!';
                    }
                    return ActionFactory::show_title_dialog($msg);
                case 'format':
                    $plugin_cookies->format = $new_value;
                    break;
            }
        }

        return ActionFactory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

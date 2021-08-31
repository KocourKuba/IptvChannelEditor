<?php
///////////////////////////////////////////////////////////////////////////
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class StarnetSetupScreen extends AbstractControlsScreen
{
    const ID = 'setup';
    const EPG_FONTSIZE_DEF_VALUE = 'normal';

    ///////////////////////////////////////////////////////////////////////
    protected $tv;
    protected $config;

    public function __construct(Tv $tv)
    {
        parent::__construct(self::ID);

        $this->tv = $tv;
        $this->config = $tv->get_config();
    }

    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    public function do_get_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $format        = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        $channels_list = isset($plugin_cookies->channels_list) ? $plugin_cookies->channels_list : $this->config->GET_CHANNEL_LIST_URL();
        $epg_font_size = isset($plugin_cookies->epg_font_size) ? $plugin_cookies->epg_font_size : self::EPG_FONTSIZE_DEF_VALUE;
        $show_tv       = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        $buf_time      = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000;
        $epg_prev      = isset($plugin_cookies->epg_prev) ? $plugin_cookies->epg_prev : 7;
        $epg_next      = isset($plugin_cookies->epg_next) ? $plugin_cookies->epg_next : 7;

        $show_ops = array();
        $show_ops['yes'] = 'Да';
        $show_ops['no'] = 'Нет';

        $channels = array();
        $list = glob(DuneSystem::$properties['install_dir_path'] . "/*.xml");
        foreach ($list as $filename) {
            $filename = basename($filename);
            if ($filename != 'dune_plugin.xml')
                $channels[$filename] = $filename;
        }

        ControlFactory::add_vgap($defs, -10);
        $title = $this->config->GET_PLUGIN_NAME() . ' v.' . $this->config->GET_PLUGIN_VERSION() . '. [' . $this->config->GET_PLUGIN_DATE() . ']';
        $this->add_button($defs, 'restart', $title, 'Перезагрузить плеер', 0);

        $this->add_combobox($defs,'show_tv', 'Показывать ' . $this->config->GET_PLUGIN_NAME() .' в главном меню:',
            $show_tv, $show_ops, 0, true);

        if (PLUGIN_TYPE === 'SharavozPluginConfig') {
            $format_ops = array();
            $format_ops['hls'] = 'HLS';
            $format_ops['mpeg'] = 'MPEG-TS';
            $this->add_combobox($defs, 'format', 'Выбор потока:', $format, $format_ops, 0, true);
        }

        $this->add_button($defs, 'key_dialog', 'Активировать просмотр:', 'Ввести ОТТ ключ и домен', 0);

        $this->add_combobox($defs,'channels_list', 'Используемый список каналов:',
            $channels_list, $channels, 0, true);

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

        $epg_prefetch = array();
        $epg_prefetch[0] = '0';
        $epg_prefetch[1] = '1';
        $epg_prefetch[2] = '2';
        $epg_prefetch[3] = '3';
        $epg_prefetch[4] = '4';
        $epg_prefetch[5] = '5';
        $epg_prefetch[6] = '6';
        $epg_prefetch[7] = '7';

        $this->add_combobox($defs, 'epg_prev', 'Предыдущие дни EPG:',
            $epg_prev, $epg_prefetch, 0, true);

        $this->add_combobox($defs, 'epg_next', 'Следующие дни EPG:',
            $epg_next, $epg_prefetch, 0, true);

        $epg_font_size_ops = array();
        $epg_font_size_ops ['normal'] = 'Обычный';
        $epg_font_size_ops ['small'] = 'Мелкий';
        $this->add_combobox($defs, 'epg_font_size', 'Размер шрифта EPG:',
            $epg_font_size, $epg_font_size_ops, 700, true);

        $this->add_button($defs, 'pass_dialog', 'Пароль для взрослых каналов:', 'Изменить пароль', 0);
        ControlFactory::add_vgap($defs, -10);

        return $defs;
    }

    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs($plugin_cookies);
    }

    public function do_get_key_control_defs(&$plugin_cookies)
    {
        $defs = array();
        $ott_key = isset($plugin_cookies->ott_key) ? $plugin_cookies->ott_key : '';

        $this->add_text_field($defs,'ott_key', 'Введите ОТТ ключ:',
            $ott_key, false, false, false, true, 500);

        $subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : '';

        $this->add_text_field($defs,'subdomain', 'Введите домен:',
            $subdomain, false, false, false, true, 500);

        $this->add_close_dialog_and_apply_button($defs,'ott_key_apply', 'ОК', 200);
        $this->add_close_dialog_button($defs,'Отмена', 200);
        return $defs;
    }

    public function do_get_pass_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $pass1 = '';
        $pass2 = '';

        $this->add_text_field($defs,'pass1', 'Старый пароль:',
            $pass1, 1, 1, 0, 1, 500, 0, false);
        $this->add_text_field($defs,'pass2', 'Новый пароль:',
            $pass2, 1, 1, 0, 1, 500, 0, false);

        $this->add_label($defs, '', '');

        $this->add_close_dialog_and_apply_button($defs,'pass_apply', 'ОК', 250);
        $this->add_close_dialog_button($defs,'Отмена', 250);

        return $defs;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        if (isset($user_input->action_type) && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $control_id = $user_input->control_id;
            $new_value = $user_input->{$control_id};
            hd_print("Setup: changing $control_id value to $new_value");

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
                case 'epg_prev':
                    $plugin_cookies->epg_prev = $new_value;
                    break;
                case 'epg_next':
                    $plugin_cookies->epg_next = $new_value;
                    break;
                case 'epg_font_size':
                    $plugin_cookies->epg_font_size = $new_value;
                    break;
                case 'key_dialog':
                    $defs = $this->do_get_key_control_defs($plugin_cookies);
                    $msg = 'Ключ чувствителен к регистру. Переключение регистра кнопкой Select';
                    return ActionFactory::show_dialog($msg, $defs, true);
                case 'ott_key_apply':
                    $plugin_cookies->ott_key = $user_input->ott_key;
                    $plugin_cookies->subdomain = $user_input->subdomain;
                    break;
                case 'pass_dialog':
                    $defs = $this->do_get_pass_control_defs($plugin_cookies);
                    $msg = 'Родительский контроль';
                    return ActionFactory::show_dialog($msg, $defs, true);
                case 'pass_apply':
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

        hd_print('Setup: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("$key => $value");

        return ActionFactory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

///////////////////////////////////////////////////////////////////////////
?>

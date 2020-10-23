<?php
///////////////////////////////////////////////////////////////////////////
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class DemoSetupScreen extends AbstractControlsScreen
{
    const ID = 'setup';
        const EPG_FONTSIZE_DEF_VALUE	= 'normal';
	private	$epg_font_size;

    ///////////////////////////////////////////////////////////////////////

    public function __construct()
    {
        parent::__construct(self::ID);
    }

    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    public function do_get_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $epg_font_size = isset($plugin_cookies->epg_font_size) ?  $plugin_cookies->epg_font_size : self::EPG_FONTSIZE_DEF_VALUE;
        $show_tv = isset($plugin_cookies->show_tv) ? $plugin_cookies->show_tv : 'yes';
        $ott_key = isset($plugin_cookies->ott_key) ? $plugin_cookies->ott_key : '';
        $show_vod = isset($plugin_cookies->show_vod) ? $plugin_cookies->show_vod : 'yes';
        $epg_shift = isset($plugin_cookies->epg_shift) ? $plugin_cookies->epg_shift : '0';
        $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 0;
        $pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
//	$country_server = isset($plugin_cookies->country_server) ? $plugin_cookies->country_server : '';
        
        $show_ops = array();
        $show_ops['yes'] = 'Да';
        $show_ops['no'] = 'Нет';

//        $server_ops = array();
//        $server_ops['iptv7.zargacum'] = 'Не выбрано';
//        $server_ops['iptv4.zargacum'] = 'Сервер EN1';
//        $server_ops['iptv5.zargacum'] = 'Сервер FR1';
//        $server_ops['iptv6.zargacum'] = 'Сервер FR2';
//        $server_ops['iptv1.zargacum'] = 'Сервер EN';
//        $server_ops['iptv2.zargacum'] = 'Сервер NL';
//        $server_ops['iptv3.zargacum'] = 'Сервер US';

        for ($i = -12; $i<13; $i++)
		    $shift_ops[$i*3600] = $i;
        ControlFactory::add_vgap($defs, -10);
        $this->add_label($defs, 'ЄDЄM TV', 'Версия '.DemoConfig::PluginVersion.'. ['.DemoConfig::PluginDate.']');

        $this->add_label($defs, 'Настройки ----------------------------------', '-----------------------------');

        $this->add_combobox($defs,
            'show_tv', 'Показывать ЄDЄM TV в главном меню:',
            $show_tv, $show_ops, 0, true);

//        $this->add_combobox($defs,
//            'country_server', 'Выбор сервера:',
//            $country_server, $server_ops, 0, true);

        $this->add_combobox($defs,
            'epg_shift', 'Коррекция программы (час):',
            $epg_shift, $shift_ops, 0, true);


            $show_buf_time_ops = array();

            $show_buf_time_ops[0] = 'По умолчанию';
            $show_buf_time_ops[500] = '0.5 с';
            $show_buf_time_ops[1000] = '1 с';
            $show_buf_time_ops[2000] = '2 с';
            $show_buf_time_ops[3000] = '3 с';
            $show_buf_time_ops[5000] = '5 с';
            $show_buf_time_ops[10000] = '10 с';

            $this->add_combobox
            (
                $defs,
                'buf_time',
                'Время буферизации:',
                $buf_time, $show_buf_time_ops, 0, true);

                $epg_font_size_ops = array();
			$epg_font_size_ops ['normal'] = 'Обычный';
			$epg_font_size_ops ['small'] = 'Мелкий';
			$this->add_combobox($defs,
				'epg_font_size', 'Размер шрифта EPG:',
				$epg_font_size, $epg_font_size_ops, 700, true);

        $this->add_button ( $defs, 'pass_dialog', 'Пароль для взрослых каналов:', 'Изменить пароль', 0);
        $this->add_button ( $defs, 'key_dialog', 'Активировать просмотр:', 'Ввести ОТТ ключ', 0);
		$this->add_button ( $defs, 'subdomain_dialog', 'Свой домен:', 'Ввести домен', 0);
//        $this->add_label($defs, 'Информация --------------------------------','-----------------------------');
        $this->add_button ( $defs, 'restart', '','Перезагрузить плеер', 0 );
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

        $this->add_text_field($defs,
            'ott_key', 'Введите ОТТ ключ:',
            $ott_key, false, false, false, true, 500);

        $this->add_close_dialog_and_apply_button($defs,
            'ott_key_apply', 'ОК', 200);
        $this->add_close_dialog_button($defs,
            'Отмена', 200);
       return $defs;
    }

    public function do_get_subdomain_control_defs(&$plugin_cookies)	
{
        $defs = array();
		$subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : '';
		$this->add_text_field($defs,
        'subdomain', 'Введите поддомен:',
        $subdomain, false, false, false, true, 500);

		$this->add_close_dialog_and_apply_button($defs,
            'subdomain_apply', 'ОК', 200);
        $this->add_close_dialog_button($defs,
            'Отмена', 200);			
       return $defs;
    }
	
	
	
    public function do_get_pass_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $pass1 = '';
        $pass2 = '';

        $this->add_text_field($defs,
            'pass1', 'Старый пароль:',
            $pass1, 1, 1, 0, 1, 500, 0, false);
        $this->add_text_field($defs,
            'pass2', 'Новый пароль:',
            $pass2, 1, 1, 0, 1, 500, 0, false);

        $this->add_label($defs, '', '');

        $this->add_close_dialog_and_apply_button($defs,
            'pass_apply', 'ОК', 250);
        $this->add_close_dialog_button($defs,
            'Отмена', 250);

        return $defs;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Setup: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("  $key => $value");

        if ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply' )

        {
            $control_id = $user_input->control_id;
            $new_value = $user_input->{$control_id};
            hd_print("Setup: changing $control_id value to $new_value");

            if ($control_id === 'restart')
                shell_exec('killall shell');
            if ($control_id === 'show_tv')
                $plugin_cookies->show_tv = $new_value;
//            else if ($control_id === 'country_server')
//                $plugin_cookies->country_server = $new_value;
            else if ($control_id === 'show_vod')
                $plugin_cookies->show_vod = $new_value;
            else if ($control_id === 'buf_time')
                $plugin_cookies->buf_time = $new_value;
            else if ($control_id === 'epg_shift')
                $plugin_cookies->epg_shift = $new_value;
            else if ($control_id == 'epg_font_size')
	        $plugin_cookies->epg_font_size = $new_value;
            else if ($control_id === 'key_dialog')
		{
		$defs = $this->do_get_key_control_defs($plugin_cookies);

		return  ActionFactory::show_dialog( "Ключ чувствителен к регистру. Переключение регистра кнопкой Select", $defs, true );

		}
            else if ($control_id === 'ott_key_apply')  {
                $ott_key = isset($plugin_cookies->ott_key) ? $plugin_cookies->ott_key : '';
                $plugin_cookies->ott_key = $user_input->ott_key;

//		return  ActionFactory::show_title_dialog($msg, $action);
		}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		else if ($control_id === 'subdomain_dialog')
		{
		$defs = $this->do_get_subdomain_control_defs($plugin_cookies);

return  ActionFactory::show_dialog( "Введите или измените домен полностью, например, dunehd.iptvspy.net", $defs, true );

	}
          else if ($control_id === 'subdomain_apply')  {
              $subdomain = isset($plugin_cookies->subdomain) ? $plugin_cookies->subdomain : '';
              $plugin_cookies->subdomain = $user_input->subdomain;

//		return  ActionFactory::show_title_dialog($msg, $action);
		}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////



        else if ($control_id === 'pass_dialog')
		{
		$defs = $this->do_get_pass_control_defs($plugin_cookies);

		return  ActionFactory::show_dialog( "Родительский контроль", $defs, true );
		}
          else if ($control_id === 'pass_apply')
		{
			if ($user_input->pass1 == '' || $user_input->pass2 == '')
				return null;
			$msg = '';
			$action = null;
			$pass_sex = isset($plugin_cookies->pass_sex) ? $plugin_cookies->pass_sex : '0000';
			if ($user_input->pass1 == $pass_sex)
				{
				$plugin_cookies->pass_sex = $user_input->{'pass2'};
				$msg = 'Пароль изменен!';
				}
			else
				{
				$msg = 'Пароль не изменен!';
				}

		return  ActionFactory::show_title_dialog($msg, $action);
		}
        }
        return ActionFactory::reset_controls(
            $this->do_get_control_defs($plugin_cookies));
    }
}

///////////////////////////////////////////////////////////////////////////
?>

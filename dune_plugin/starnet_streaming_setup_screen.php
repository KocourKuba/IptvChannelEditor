<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Streaming_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'stream_setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_AUTO_RESUME = 'auto_resume';
    const SETUP_ACTION_AUTO_PLAY = 'auto_play';
    const SETUP_ACTION_SERVER = 'server';
    const SETUP_ACTION_DEVICE = 'device';
    const SETUP_ACTION_PROFILE = 'profile';
    const SETUP_ACTION_QUALITY = 'quality';
    const SETUP_ACTION_STREAM_FORMAT = 'stream_format';
    const SETUP_ACTION_BUF_TIME = 'buf_time';
    const SETUP_ACTION_DELAY_TIME = 'delay_time';

    private static $on_off_ops = array(
        SetupControlSwitchDefs::switch_on => '%tr%yes',
        SetupControlSwitchDefs::switch_off => '%tr%no',
    );

    private static $on_off_img = array(
        SetupControlSwitchDefs::switch_on => 'on.png',
        SetupControlSwitchDefs::switch_off => 'off.png',
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
     * streaming parameters dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_control_defs(&$plugin_cookies)
    {
        $defs = array();

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // auto play
        if (!isset($plugin_cookies->auto_play))
            $plugin_cookies->auto_play = SetupControlSwitchDefs::switch_off;

        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_AUTO_PLAY, TR::t('setup_autostart'), self::$on_off_ops[$plugin_cookies->auto_play],
            $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->auto_play]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // auto resume
        if (!isset($plugin_cookies->auto_resume))
            $plugin_cookies->auto_resume = SetupControlSwitchDefs::switch_on;

        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_AUTO_RESUME, TR::t('setup_continue_play'),  self::$on_off_ops[$plugin_cookies->auto_resume],
            $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->auto_resume]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // select server
        $servers = $this->plugin->config->get_servers($plugin_cookies);
        if (!empty($servers)) {
            hd_print("Change server supported");
            $server_id = $this->plugin->config->get_server_id($plugin_cookies);
            $server_name = $this->plugin->config->get_server_name($plugin_cookies);
            hd_print("Selected server: id: $server_id name: '$server_name'");
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_SERVER, TR::t('server'),
                $server_id, $servers, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // select device number
        $devices = $this->plugin->config->get_devices($plugin_cookies);
        if (!empty($devices)) {
            hd_print("Change device supported");
            $device_id = $this->plugin->config->get_device_id($plugin_cookies);
            $device_name = $this->plugin->config->get_device_name($plugin_cookies);
            hd_print("Selected device: id: $device_id name: '$device_name'");
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_DEVICE, TR::t('setup_device'),
                $device_id, $devices,self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // select quality
        $qualities = $this->plugin->config->get_qualities($plugin_cookies);
        if (!empty($qualities)) {
            hd_print("Change quality supported");
            $quality_id = $this->plugin->config->get_quality_id($plugin_cookies);
            $quality_name = $this->plugin->config->get_quality_name($plugin_cookies);
            hd_print("Selected quality: id: $quality_id name: '$quality_name'");
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_QUALITY, TR::t('setup_quality'),
                $quality_id, $qualities, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // select profile
        $profiles = $this->plugin->config->get_profiles($plugin_cookies);
        if (!empty($profiles)) {
            hd_print("Change profile supported");
            $profile_id = $this->plugin->config->get_profile_id($plugin_cookies);
            $profile_name = $this->plugin->config->get_profile_name($plugin_cookies);
            hd_print("Selected profile: id: $profile_id name: '$profile_name'");
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_PROFILE, TR::t('setup_profile'),
                $profile_id, $profiles, self::CONTROLS_WIDTH, true);
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
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_STREAM_FORMAT, TR::t('setup_stream'),
                $format_id, $format_ops, self::CONTROLS_WIDTH, true);
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
        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_BUF_TIME, TR::t('setup_buffer_time'),
            $buf_time, $show_buf_time_ops, self::CONTROLS_WIDTH, true);

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
        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_DELAY_TIME, TR::t('setup_delay_time'),
            $delay_time, $show_delay_time_ops, self::CONTROLS_WIDTH, true);

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

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        $control_id = $user_input->control_id;
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            hd_print(__METHOD__ . ": Setup: changing $control_id value to $new_value");
        }

        $need_reload = false;
        switch ($control_id) {
            case self::SETUP_ACTION_AUTO_PLAY:
                self::toggle_param($plugin_cookies, self::SETUP_ACTION_AUTO_PLAY);
                hd_print(__METHOD__ . ": Auto play: $plugin_cookies->auto_play");
                break;

            case self::SETUP_ACTION_AUTO_RESUME:
                self::toggle_param($plugin_cookies, self::SETUP_ACTION_AUTO_RESUME);
                hd_print(__METHOD__ . ": Auto resume: $plugin_cookies->auto_resume");
                break;

            case self::SETUP_ACTION_SERVER:
                if (isset($user_input->server) && $this->plugin->config->get_server_id($plugin_cookies) !== $user_input->server) {
                    $need_reload = true;
                    $this->plugin->config->set_server_id($user_input->server, $plugin_cookies);
                    hd_print(__METHOD__ . ": Selected server: id: $user_input->server name: '" . $this->plugin->config->get_server_name($plugin_cookies) . "'");
                }
                break;

            case self::SETUP_ACTION_DEVICE:
                if (isset($user_input->device) && $this->plugin->config->get_device_id($plugin_cookies) !== $user_input->device) {
                    $need_reload = true;
                    $this->plugin->config->set_device_id($user_input->device, $plugin_cookies);
                    hd_print(__METHOD__ . ": Selected device: id: $user_input->device name: '" . $this->plugin->config->get_device_name($plugin_cookies) . "'");
                }
                break;

            case self::SETUP_ACTION_PROFILE:
                if (isset($user_input->profile) && $this->plugin->config->get_profile_id($plugin_cookies) !== $user_input->profile) {
                    $need_reload = true;
                    $this->plugin->config->set_profile_id($user_input->profile, $plugin_cookies);
                    hd_print(__METHOD__ . ": Selected profile: id: $user_input->profile name: '" . $this->plugin->config->get_profile_name($plugin_cookies) . "'");
                }
                break;

            case self::SETUP_ACTION_QUALITY:
                if (isset($user_input->quality) && $this->plugin->config->get_quality_id($plugin_cookies) !== $user_input->quality) {
                    $need_reload = true;
                    $this->plugin->config->set_quality_id($user_input->quality, $plugin_cookies);
                    hd_print(__METHOD__ . ": Selected quality: id: $user_input->quality name: '" . $this->plugin->config->get_quality_name($plugin_cookies) . "'");
                }
                break;

            case self::SETUP_ACTION_STREAM_FORMAT:
                if (isset($user_input->stream_format)) {
                    $need_reload = true;
                    $plugin_cookies->format = $user_input->stream_format;
                    hd_print(__METHOD__ . ": Selected stream type: $plugin_cookies->format");
                }
                break;

            case self::SETUP_ACTION_BUF_TIME:
                $plugin_cookies->buf_time = $user_input->buf_time;
                hd_print(__METHOD__ . ": Buffering time: $plugin_cookies->buf_time");
                break;

            case self::SETUP_ACTION_DELAY_TIME:
                $plugin_cookies->delay_time = $user_input->delay_time;
                hd_print(__METHOD__ . ": Delay time: $plugin_cookies->delay_time");
                break;
        }

        if ($need_reload) {
            return $this->plugin->tv->reload_channels($this, $plugin_cookies);
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }

    private static function toggle_param($plugin_cookies, $param)
    {
        $plugin_cookies->{$param} = ($plugin_cookies->{$param} === SetupControlSwitchDefs::switch_off)
            ? SetupControlSwitchDefs::switch_on
            : SetupControlSwitchDefs::switch_off;
    }
}

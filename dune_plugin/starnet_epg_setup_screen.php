<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Epg_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'epg_setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_EPG_SOURCE = 'epg_source';
    const SETUP_ACTION_CLEAR_EPG_CACHE = 'clear_epg_cache';
    const SETUP_ACTION_EPG_FONT = 'epg_font_size';
    const SETUP_ACTION_EPG_SHIFT = 'epg_shift';

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_small => '%tr%setup_small',
        SetupControlSwitchDefs::switch_normal => '%tr%setup_normal',
        SetupControlSwitchDefs::switch_epg1 => '%tr%setup_first',
        SetupControlSwitchDefs::switch_epg2 => '%tr%setup_second',
    );

    private static $on_off_img = array
    (
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
     * EPG dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_control_defs(&$plugin_cookies)
    {
        $defs = array();

        $remove_icon = $this->plugin->get_image_path('brush.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // EPG
        $epg_params = $this->plugin->config->get_epg_params(Plugin_Constants::EPG_SECOND);
        if (!empty($epg_params[Epg_Params::EPG_URL])) {
            $epg_source_ops = array();
            $epg_source_ops[SetupControlSwitchDefs::switch_epg1] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg1];
            $epg_source_ops[SetupControlSwitchDefs::switch_epg2] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg2];

            if (!isset($plugin_cookies->epg_source)) {
                $plugin_cookies->epg_source = SetupControlSwitchDefs::switch_epg1;
            }

            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_EPG_SOURCE, TR::t('setup_epg_source'), $plugin_cookies->epg_source, $epg_source_ops, self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_CLEAR_EPG_CACHE, TR::t('entry_epg_cache_clear'), TR::t('clear'), $remove_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // epg font size
        if (!isset($plugin_cookies->epg_font_size)) {
            $plugin_cookies->epg_font_size = SetupControlSwitchDefs::switch_normal;
        }

        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_EPG_FONT, TR::t('setup_epg_font'), self::$on_off_ops[$plugin_cookies->epg_font_size],
            $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->epg_font_size]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // epg time shift
        $show_epg_shift_ops = array();
        for ($i = -11; $i < 12; $i++) {
            $show_epg_shift_ops[$i] = TR::t('setup_epg_shift__1', sprintf("%+03d", $i));
        }
        $show_epg_shift_ops[0] = TR::t('setup_epg_shift_default__1', "00");

        if (!isset($plugin_cookies->epg_shift)) {
            $plugin_cookies->epg_shift = 0;
        }
        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_EPG_SHIFT, TR::t('setup_epg_shift'), $plugin_cookies->epg_shift, $show_epg_shift_ops, self::CONTROLS_WIDTH);

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

        switch ($control_id) {
            case self::SETUP_ACTION_EPG_SOURCE:
                $plugin_cookies->epg_source = $user_input->epg_source;
                hd_print(__METHOD__ . ": Selected epg source: $plugin_cookies->epg_source");
                break;

            case self::SETUP_ACTION_EPG_FONT: // handle epg settings dialog result
                $plugin_cookies->epg_font_size = ($plugin_cookies->epg_font_size === SetupControlSwitchDefs::switch_small)
                    ? SetupControlSwitchDefs::switch_normal
                    : SetupControlSwitchDefs::switch_small;
                hd_print(__METHOD__ . ": Selected epg font size: $plugin_cookies->epg_font_size");
                break;

            case self::SETUP_ACTION_EPG_SHIFT: // handle epg settings dialog result
                $plugin_cookies->epg_shift = $user_input->epg_shift;
                hd_print(__METHOD__ . ": Selected epg shift: $plugin_cookies->epg_shift");
                break;

            case self::SETUP_ACTION_CLEAR_EPG_CACHE: // clear epg cache
                $epg_path = get_temp_path("epg/");
                hd_print(__METHOD__ . ": do clear epg: $epg_path");
                foreach(glob($epg_path . "*") as $file) {
                    if(is_file($file)) {
                        hd_print("erase: $file");
                        unlink($file);
                    }
                }

                return Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

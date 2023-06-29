<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Epg_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'epg_setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_EPG_SOURCE = 'epg_source';
    const SETUP_ACTION_XMLTV_EPG_IDX = 'xmltv_epg_idx';
    const SETUP_ACTION_CUSTOM_XMLTV_EPG = 'custom_xmltv_epg';
    const SETUP_ACTION_CUSTOM_XMLTV_EPG_DLG = 'xmltv_custom_epg_dlg';
    const SETUP_ACTION_CUSTOM_XMLTV_EPG_APPLY = 'xmltv_custom_epg_apply';
    const SETUP_ACTION_EPG_CACHE_TTL = 'epg_cache_ttl';
    const SETUP_ACTION_CLEAR_EPG_CACHE = 'clear_epg_cache';
    const SETUP_ACTION_EPG_FONT_SIZE = 'epg_font_size';
    const SETUP_ACTION_EPG_SHIFT = 'epg_shift';

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_small => '%tr%setup_small',
        SetupControlSwitchDefs::switch_normal => '%tr%setup_normal',
        SetupControlSwitchDefs::switch_epg1 => '%tr%setup_first',
        SetupControlSwitchDefs::switch_epg2 => '%tr%setup_second',
        SetupControlSwitchDefs::switch_epg3 => 'XMLTV',
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
        $epg_source_ops = array();
        $epg_source_ops[SetupControlSwitchDefs::switch_epg1] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg1];
        $epg_source_ops[SetupControlSwitchDefs::switch_epg2] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg2];
        $epg_source_ops[SetupControlSwitchDefs::switch_epg3] = self::$on_off_ops[SetupControlSwitchDefs::switch_epg3];

        if (!isset($plugin_cookies->{self::SETUP_ACTION_EPG_SOURCE})) {
            $plugin_cookies->{self::SETUP_ACTION_EPG_SOURCE} = SetupControlSwitchDefs::switch_epg1;
        }

        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_EPG_SOURCE, TR::t('setup_epg_source'),
            $plugin_cookies->{self::SETUP_ACTION_EPG_SOURCE}, $epg_source_ops, self::CONTROLS_WIDTH, true);

        //////////////////////////////////////
        // XMLTV EPG source
        if ($plugin_cookies->{self::SETUP_ACTION_EPG_SOURCE} === SetupControlSwitchDefs::switch_epg3) {
            $xmltv_epg_idx = isset($plugin_cookies->{self::SETUP_ACTION_XMLTV_EPG_IDX}) ? $plugin_cookies->{self::SETUP_ACTION_XMLTV_EPG_IDX} : 'custom';

            $urls = $this->plugin->config->epg_man->get_xmltv_urls();
            if (empty($urls['custom'])) {
                $urls['custom'] = TR::t('custom');
            }
            Control_Factory::add_combobox($defs, $this, null,
                self::SETUP_ACTION_XMLTV_EPG_IDX, TR::t('setup_xmltv_epg_source'),
                $xmltv_epg_idx, $urls, self::CONTROLS_WIDTH, true);

            if ($xmltv_epg_idx === 'custom') {
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CUSTOM_XMLTV_EPG_DLG,
                    TR::t('setup_xmltv_set_epg_source'), TR::t('setup_enter_xmltv_url'), $this->plugin->get_image_path('text.png'));
            }
        }

        //////////////////////////////////////
        // EPG cache
        $epg_cache_ops = array();
        $epg_cache_ops[1] = 1;
        $epg_cache_ops[2] = 2;
        $epg_cache_ops[3] = 3;
        $epg_cache_ops[5] = 5;
        $epg_cache_ops[7] = 7;

        if (!isset($plugin_cookies->{self::SETUP_ACTION_EPG_CACHE_TTL})) {
            $plugin_cookies->{self::SETUP_ACTION_EPG_CACHE_TTL} = 3;
        }

        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_EPG_CACHE_TTL, TR::t('setup_epg_cache_ttl'),
            $plugin_cookies->{self::SETUP_ACTION_EPG_CACHE_TTL}, $epg_cache_ops, self::CONTROLS_WIDTH, true);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_CLEAR_EPG_CACHE, TR::t('entry_epg_cache_clear'), TR::t('clear'),
            $remove_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // epg font size
        if (!isset($plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE})) {
            $plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE} = SetupControlSwitchDefs::switch_normal;
        }

        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_EPG_FONT_SIZE, TR::t('setup_epg_font'), self::$on_off_ops[$plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE}],
            $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE}]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // epg time shift
        $show_epg_shift_ops = array();
        for ($i = -11; $i < 12; $i++) {
            $show_epg_shift_ops[$i] = TR::t('setup_epg_shift__1', sprintf("%+03d", $i));
        }
        $show_epg_shift_ops[0] = TR::t('setup_epg_shift_default__1', "00");

        if (!isset($plugin_cookies->{self::SETUP_ACTION_EPG_SHIFT})) {
            $plugin_cookies->{self::SETUP_ACTION_EPG_SHIFT} = 0;
        }
        Control_Factory::add_combobox($defs, $this, null,
            self::SETUP_ACTION_EPG_SHIFT, TR::t('setup_epg_shift'),
            $plugin_cookies->{self::SETUP_ACTION_EPG_SHIFT}, $show_epg_shift_ops, self::CONTROLS_WIDTH);

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
    public function do_get_custom_epg_defs(&$plugin_cookies)
    {
        $defs = array();
        $custom_xmltv_epg = isset($plugin_cookies->{self::SETUP_ACTION_CUSTOM_XMLTV_EPG}) ? $plugin_cookies->{self::SETUP_ACTION_CUSTOM_XMLTV_EPG} : '';

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, self::SETUP_ACTION_CUSTOM_XMLTV_EPG, TR::t('setup_enter_xmltv_url'),
            $custom_xmltv_epg, false, false, false, true, 600);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_CUSTOM_XMLTV_EPG_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
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
                $val = $user_input->{self::SETUP_ACTION_EPG_SOURCE};
                $plugin_cookies->{self::SETUP_ACTION_EPG_SOURCE} = $val;
                hd_print(__METHOD__ . ": Selected epg source: $val");
                if ($val === SetupControlSwitchDefs::switch_epg3) {
                    $this->plugin->config->epg_man->xmltv_data = null;
                    return $this->plugin->tv->reload_channels($this, $plugin_cookies);
                }
                break;

            case self::SETUP_ACTION_XMLTV_EPG_IDX:
                $val = $user_input->{self::SETUP_ACTION_XMLTV_EPG_IDX};
                $plugin_cookies->{self::SETUP_ACTION_XMLTV_EPG_IDX} = $val;
                hd_print(__METHOD__ . ": Selected xmltv epg idx: $val");
                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

            case self::SETUP_ACTION_CUSTOM_XMLTV_EPG_DLG: // show ott key dialog
                $defs = $this->do_get_custom_epg_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_enter_xmltv_epg_source'), $defs, true);

            case self::SETUP_ACTION_CUSTOM_XMLTV_EPG_APPLY: // handle ott key dialog result
                $val = $user_input->{self::SETUP_ACTION_CUSTOM_XMLTV_EPG};
                $plugin_cookies->{self::SETUP_ACTION_CUSTOM_XMLTV_EPG} = $val;
                $this->plugin->config->epg_man->set_xmltv_url('custom', $val);
                hd_print(__METHOD__ . ": Selected custom xmltv epg source: $val");
                $this->plugin->config->epg_man->xmltv_data = null;
                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

            case self::SETUP_ACTION_EPG_CACHE_TTL:
                $val = $user_input->{self::SETUP_ACTION_EPG_CACHE_TTL};
                $plugin_cookies->{self::SETUP_ACTION_EPG_CACHE_TTL} = $val;
                hd_print(__METHOD__ . ": Selected epg cache ttl: $val");
                break;

            case self::SETUP_ACTION_EPG_FONT_SIZE: // handle epg settings dialog result
                $plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE} = ($plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE} === SetupControlSwitchDefs::switch_small)
                    ? SetupControlSwitchDefs::switch_normal
                    : SetupControlSwitchDefs::switch_small;
                hd_print(__METHOD__ . ": Selected epg font size: " . $plugin_cookies->{self::SETUP_ACTION_EPG_FONT_SIZE});
                break;

            case self::SETUP_ACTION_EPG_SHIFT: // handle epg settings dialog result
                $val = $user_input->{self::SETUP_ACTION_EPG_SHIFT};
                $plugin_cookies->{self::SETUP_ACTION_EPG_SHIFT} = $val;
                hd_print(__METHOD__ . ": Selected epg shift: $val");
                break;

            case self::SETUP_ACTION_CLEAR_EPG_CACHE: // clear epg cache
                $this->plugin->config->epg_man->clear_epg_cache();
                return Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}
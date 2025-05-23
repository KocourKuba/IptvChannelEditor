<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Interface_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'interface_setup';

    const CONTROL_SHOW_TV = 'show_tv';

    ///////////////////////////////////////////////////////////////////////

    /**
     * interface dialog defs
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
        // Show in main screen
        if (!is_limited_apk()) {
            $show_tv = self::get_cookie_bool_param($plugin_cookies, self::CONTROL_SHOW_TV);
            Control_Factory::add_image_button($defs, $this, null,
                self::CONTROL_SHOW_TV, TR::t('setup_show_in_main'), SwitchOnOff::$translated[$show_tv],
                get_image_path(SwitchOnOff::$image[$show_tv]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // ask exit parameter
        $ask_exit = $this->plugin->get_parameter(PARAM_ASK_EXIT, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_ASK_EXIT, TR::t('setup_ask_exit'), SwitchOnOff::$translated[$ask_exit],
            get_image_path(SwitchOnOff::$image[$ask_exit]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // show all channels category
        $show_all = $this->plugin->get_parameter(PARAM_SHOW_ALL, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_SHOW_ALL, TR::t('setup_show_all_channels'), SwitchOnOff::$translated[$show_all],
            get_image_path(SwitchOnOff::$image[$show_all]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // show favorites category
        $show_fav = $this->plugin->get_parameter(PARAM_SHOW_FAVORITES, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_SHOW_FAVORITES, TR::t('setup_show_favorites'), SwitchOnOff::$translated[$show_fav],
            get_image_path(SwitchOnOff::$image[$show_fav]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // show history category
        $show_history = $this->plugin->get_parameter(PARAM_SHOW_HISTORY, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_SHOW_HISTORY, TR::t('setup_show_history'), SwitchOnOff::$translated[$show_history],
            get_image_path(SwitchOnOff::$image[$show_history]), self::CONTROLS_WIDTH);

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) !== "None") {
            //////////////////////////////////////
            // show history category
            $show_vod = $this->plugin->get_parameter(PARAM_SHOW_VOD, SwitchOnOff::on);
            Control_Factory::add_image_button($defs, $this, null,
                PARAM_SHOW_VOD, TR::t('setup_show_vod'), SwitchOnOff::$translated[$show_vod],
                get_image_path(SwitchOnOff::$image[$show_vod]), self::CONTROLS_WIDTH);

            if ($show_vod) {            //////////////////////////////////////
                // show vod at the end of categories
                $vod_last = $this->plugin->get_parameter(PARAM_VOD_LAST, SwitchOnOff::off);
                Control_Factory::add_image_button($defs, $this, null,
                    PARAM_VOD_LAST, TR::t('setup_vod_last'), SwitchOnOff::$translated[$vod_last],
                    get_image_path(SwitchOnOff::$image[$vod_last]), self::CONTROLS_WIDTH);
            }
        }

        //////////////////////////////////////
        // epg font size
        $font_size = $this->plugin->get_parameter(PARAM_EPG_FONT_SIZE, SwitchOnOff::on);
        $font_ops_translated[SwitchOnOff::on] = TR::t('setup_small');
        $font_ops_translated[SwitchOnOff::off] = TR::t('setup_normal');

        Control_Factory::add_image_button($defs, $this, null,
            PARAM_EPG_FONT_SIZE, TR::t('setup_epg_font'), $font_ops_translated[$font_size],
            get_image_path(SwitchOnOff::$image[$font_size]), self::CONTROLS_WIDTH);

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
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        $control_id = $user_input->control_id;
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            hd_debug_print("changing $control_id value to $new_value");
        }

        switch ($control_id) {
            case self::CONTROL_SHOW_TV:
                if (!is_limited_apk()) {
                    self::toggle_cookie_param($plugin_cookies, $control_id);
                }
                break;

            case PARAM_SHOW_ALL:
            case PARAM_SHOW_FAVORITES:
            case PARAM_SHOW_HISTORY:
            case PARAM_SHOW_VOD:
            case PARAM_VOD_LAST:
                $this->plugin->toggle_parameter($control_id);
                $this->plugin->tv->reload_channels();
                $this->plugin->set_need_update_epfs();
                return Action_Factory::invalidate_all_folders($plugin_cookies, Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies)));

            case PARAM_ASK_EXIT:
            case PARAM_EPG_FONT_SIZE:
                $this->plugin->toggle_parameter($control_id, false);
                break;
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

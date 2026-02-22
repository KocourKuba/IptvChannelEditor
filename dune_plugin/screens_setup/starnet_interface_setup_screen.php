<?php
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Interface_Setup_Screen extends Abstract_Controls_Screen
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
            $show_tv = get_cookie_bool_param($plugin_cookies, self::CONTROL_SHOW_TV);
            Control_Factory::add_image_button($defs, $this, self::CONTROL_SHOW_TV,
                TR::t('setup_show_in_main'), SwitchOnOff::translate($show_tv), SwitchOnOff::to_image($show_tv));
        }

        //////////////////////////////////////
        // ask exit parameter
        $ask_exit = $this->plugin->get_setting(PARAM_ASK_EXIT, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, PARAM_ASK_EXIT,
            TR::t('setup_ask_exit'), SwitchOnOff::translate($ask_exit), SwitchOnOff::to_image($ask_exit));

        //////////////////////////////////////
        // show all channels category
        $show_all = $this->plugin->get_setting(PARAM_SHOW_ALL, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, PARAM_SHOW_ALL,
            TR::t('setup_show_all_channels'), SwitchOnOff::translate($show_all), SwitchOnOff::to_image($show_all));

        //////////////////////////////////////
        // show favorites category
        $show_fav = $this->plugin->get_setting(PARAM_SHOW_FAVORITES, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, PARAM_SHOW_FAVORITES,
            TR::t('setup_show_favorites'), SwitchOnOff::translate($show_fav), SwitchOnOff::to_image($show_fav));

        //////////////////////////////////////
        // show history category
        $show_history = $this->plugin->get_setting(PARAM_SHOW_HISTORY, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, PARAM_SHOW_HISTORY,
            TR::t('setup_show_history'), SwitchOnOff::translate($show_history), SwitchOnOff::to_image($show_history));

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) !== "None") {
            //////////////////////////////////////
            // show history category
            $show_vod = $this->plugin->get_setting(PARAM_SHOW_VOD, SwitchOnOff::on);
            Control_Factory::add_image_button($defs, $this, PARAM_SHOW_VOD,
                TR::t('setup_show_vod'), SwitchOnOff::translate($show_vod), SwitchOnOff::to_image($show_vod));

            if ($show_vod) {            //////////////////////////////////////
                // show vod at the end of categories
                $vod_last = $this->plugin->get_setting(PARAM_VOD_LAST, SwitchOnOff::off);
                Control_Factory::add_image_button($defs, $this, PARAM_VOD_LAST,
                    TR::t('setup_vod_last'), SwitchOnOff::translate($vod_last), SwitchOnOff::to_image($vod_last));
            }
        }

        //////////////////////////////////////
        // epg font size
        $font_size = $this->plugin->get_setting(PARAM_EPG_FONT_SIZE, SwitchOnOff::on);
        $font_ops_translated = array(SwitchOnOff::on => TR::t('setup_small'), SwitchOnOff::off => TR::t('setup_normal'));
        Control_Factory::add_image_button($defs, $this, PARAM_EPG_FONT_SIZE,
            TR::t('setup_epg_font'), SwitchOnOff::translate_from($font_ops_translated, $font_size), SwitchOnOff::to_image($font_size));

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
        switch ($control_id) {
            case GUI_EVENT_KEY_TOP_MENU:
            case GUI_EVENT_KEY_RETURN:
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                return self::make_return_action($parent_media_url);

            case self::CONTROL_SHOW_TV:
                if (!is_limited_apk()) {
                    toggle_cookie_param($plugin_cookies, $control_id, SwitchOnOff::on);
                }
                break;

            case PARAM_SHOW_ALL:
            case PARAM_SHOW_FAVORITES:
            case PARAM_SHOW_HISTORY:
            case PARAM_SHOW_VOD:
            case PARAM_VOD_LAST:
                $this->plugin->toggle_setting($control_id);
                $this->plugin->tv->reload_channels();
                $this->plugin->set_need_update_epfs();
                return Action_Factory::invalidate_all_folders($plugin_cookies, Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies)));

            case PARAM_ASK_EXIT:
            case PARAM_EPG_FONT_SIZE:
                $this->plugin->toggle_setting($control_id, false);
                break;
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

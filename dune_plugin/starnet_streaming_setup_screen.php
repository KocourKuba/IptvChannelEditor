<?php
require_once 'lib/abstract_controls_screen.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Streaming_Setup_Screen extends Abstract_Controls_Screen
{
    const ID = 'stream_setup';

    const CONTROL_AUTO_RESUME = 'auto_resume';
    const CONTROL_AUTO_PLAY = 'auto_play';

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
        if (!isset($plugin_cookies->{self::CONTROL_AUTO_PLAY}))
            $plugin_cookies->{self::CONTROL_AUTO_PLAY} = SwitchOnOff::off;

        Control_Factory::add_image_button($defs, $this, self::CONTROL_AUTO_PLAY,
            TR::t('setup_autostart'), SwitchOnOff::translate($plugin_cookies->{self::CONTROL_AUTO_PLAY}),
            SwitchOnOff::to_image($plugin_cookies->{self::CONTROL_AUTO_PLAY})
        );

        //////////////////////////////////////
        // auto resume
        if (!isset($plugin_cookies->{self::CONTROL_AUTO_RESUME}))
            $plugin_cookies->{self::CONTROL_AUTO_RESUME} = SwitchOnOff::on;

        Control_Factory::add_image_button($defs, $this, self::CONTROL_AUTO_RESUME,
            TR::t('setup_continue_play'), SwitchOnOff::translate($plugin_cookies->{self::CONTROL_AUTO_RESUME}),
            SwitchOnOff::to_image($plugin_cookies->{self::CONTROL_AUTO_RESUME})
        );

        //////////////////////////////////////
        // Per channel zoom
        $per_channel_zoom = $this->plugin->get_setting(PARAM_PER_CHANNELS_ZOOM, SwitchOnOff::on);
        Control_Factory::add_image_button($defs, $this, PARAM_PER_CHANNELS_ZOOM,
            TR::t('setup_per_channel_zoom'), SwitchOnOff::translate($per_channel_zoom), SwitchOnOff::to_image($per_channel_zoom)
        );

        //////////////////////////////////////
        // playlist caching
        foreach (array(1, 6, 12) as $hour) {
            $caching_range[$hour] = TR::t('setup_cache_time_h__1', $hour);
        }
        foreach (array(24, 48, 96, 168) as $hour) {
            $caching_range[$hour] = TR::t('setup_cache_time_d__1', $hour / 24);
        }
        $cache_time = $this->plugin->get_setting(PARAM_PLAYLIST_CACHE_TIME, 1);
        Control_Factory::add_combobox($defs, $this, PARAM_PLAYLIST_CACHE_TIME,
            TR::t('setup_cache_time'), $cache_time, $caching_range);

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

        $buf_time = (int)$this->plugin->get_setting(PARAM_BUFFERING_TIME,1000);
        Control_Factory::add_combobox($defs, $this, PARAM_BUFFERING_TIME,
            TR::t('setup_buffer_time'), $buf_time, $show_buf_time_ops);

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

        $delay_time = (int)$this->plugin->get_setting(PARAM_ARCHIVE_DELAY_TIME,60);
        Control_Factory::add_combobox($defs, $this, PARAM_ARCHIVE_DELAY_TIME,
            TR::t('setup_delay_time'), $delay_time, $show_delay_time_ops);

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

            case self::CONTROL_AUTO_PLAY:
            case self::CONTROL_AUTO_RESUME:
                toggle_cookie_param($plugin_cookies, $control_id);
                hd_debug_print("$control_id: " . $plugin_cookies->{$control_id});
                break;

            case PARAM_BUFFERING_TIME:
            case PARAM_ARCHIVE_DELAY_TIME:
            case PARAM_PLAYLIST_CACHE_TIME:
                $this->plugin->set_setting($control_id, $user_input->{$control_id});
                hd_debug_print("$control_id: " . $user_input->{$control_id}, true);
                break;

            case PARAM_PER_CHANNELS_ZOOM:
                $this->plugin->toggle_setting(PARAM_PER_CHANNELS_ZOOM);
                break;
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

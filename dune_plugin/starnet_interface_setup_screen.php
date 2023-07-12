<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Interface_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'interface_setup';

    const SETUP_ACTION_SHOW_TV = 'show_tv';
    const SETUP_ACTION_VOD_LAST = 'vod_last';
    const SETUP_ACTION_SHOW_ALL = 'show_all';

    private static $on_off_ops = array
    (
        SetupControlSwitchDefs::switch_on => '%tr%yes',
        SetupControlSwitchDefs::switch_off => '%tr%no',
    );

    private static $on_off_img = array
    (
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
        if (!is_apk()) {
            if (!isset($plugin_cookies->show_tv)) {
                $plugin_cookies->show_tv = SetupControlSwitchDefs::switch_on;
            }
            Control_Factory::add_image_button($defs, $this, null,
                self::SETUP_ACTION_SHOW_TV, TR::t('setup_show_in_main'), self::$on_off_ops[$plugin_cookies->show_tv],
                $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->show_tv]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // show all channels category
        if (!isset($plugin_cookies->show_all)) {
            $plugin_cookies->show_all = SetupControlSwitchDefs::switch_on;
        }

        Control_Factory::add_image_button($defs, $this, null,
            self::SETUP_ACTION_SHOW_ALL, TR::t('setup_show_all_channels'), self::$on_off_ops[$plugin_cookies->show_all],
            $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->show_all]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // show vod at the end of categories
        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            if (!isset($plugin_cookies->vod_last)) {
                $plugin_cookies->vod_last = SetupControlSwitchDefs::switch_on;
            }

            Control_Factory::add_image_button($defs, $this, null,
                self::SETUP_ACTION_VOD_LAST, TR::t('setup_vod_last'), self::$on_off_ops[$plugin_cookies->vod_last],
                $this->plugin->get_image_path(self::$on_off_img[$plugin_cookies->vod_last]), self::CONTROLS_WIDTH);
        }

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
            hd_print(__METHOD__ . ": changing $control_id value to $new_value");
        }

        $need_reload = false;
        switch ($control_id) {
            case self::SETUP_ACTION_SHOW_TV:
                if (!is_apk()) {
                    self::toggle_param($plugin_cookies, self::SETUP_ACTION_SHOW_TV);
                    hd_print(__METHOD__ . ": Show on main screen: $plugin_cookies->show_tv");
                }
                break;

            case self::SETUP_ACTION_SHOW_ALL:
                self::toggle_param($plugin_cookies, self::SETUP_ACTION_SHOW_ALL);
                hd_print(__METHOD__ . ": Show all channels category: $plugin_cookies->show_all");
                $need_reload = true;
                break;

            case self::SETUP_ACTION_VOD_LAST:
                self::toggle_param($plugin_cookies, self::SETUP_ACTION_VOD_LAST);
                hd_print(__METHOD__ . ": Vod at last: $plugin_cookies->vod_last");
                $need_reload = true;
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

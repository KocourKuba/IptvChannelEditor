<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'setup';

    const SETUP_ACTION_DONATE_DLG = 'donate_dlg';
    const CONTROL_INTERFACE_SCREEN = 'interface_screen';
    const CONTROL_CHANNELS_SCREEN = 'channels_screen';
    const CONTROL_EPG_SCREEN = 'epg_screen';
    const CONTROL_STREAMING_SCREEN = 'streaming_screen';
    const CONTROL_EXT_SETUP_SCREEN = 'extended_setup_screen';

    ///////////////////////////////////////////////////////////////////////

    /**
     * defs for all controls on screen
     * @return array
     */
    public function do_get_control_defs()
    {
        $setting_icon = get_image_path('settings.png');

        $defs = array();
        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        Control_Factory::add_button($defs, $this,null, self::SETUP_ACTION_DONATE_DLG,
            TR::t('setup_donate_title'), 'QR code', self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Streaming settings
        Control_Factory::add_image_button($defs, $this, null, self::CONTROL_STREAMING_SCREEN,
            TR::t('setup_streaming_settings'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Interface settings
        Control_Factory::add_image_button($defs, $this, null, self::CONTROL_INTERFACE_SCREEN,
            TR::t('setup_interface_title'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Channels settings
        Control_Factory::add_image_button($defs, $this, null, self::CONTROL_CHANNELS_SCREEN,
            TR::t('tv_screen_channels_setup'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // EPG settings
        Control_Factory::add_image_button($defs, $this, null, self::CONTROL_EPG_SCREEN,
            TR::t('setup_epg_settings'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // Extended settings
        Control_Factory::add_image_button($defs, $this, null,
            self::CONTROL_EXT_SETUP_SCREEN,
            TR::t('setup_extended_setup'), TR::t('setup_change_settings'), $setting_icon, self::CONTROLS_WIDTH);

        return $defs;
    }

    public function do_donate_dialog()
    {
        try {
            hd_debug_print(null, true);
            $img_ym = get_temp_path('qr_ym.png');
            file_put_contents($img_ym, HD::http_get_document(Default_Dune_Plugin::RESOURCE_URL . "/QR_YM.png"));
            $img_pp = get_temp_path('qr_pp.png');
            file_put_contents($img_pp, HD::http_get_document(Default_Dune_Plugin::RESOURCE_URL . "/QR_PP.png"));

            Control_Factory::add_vgap($defs, 50);
            Control_Factory::add_smart_label($defs, "", "<text>YooMoney</text><gap width=400/><text>PayPal</text>");
            Control_Factory::add_smart_label($defs, "", "<icon>$img_ym</icon><gap width=140/><icon>$img_pp</icon>");
            Control_Factory::add_vgap($defs, 450);

            $attrs['dialog_params'] = array('frame_style' => DIALOG_FRAME_STYLE_GLASS);
            return Action_Factory::show_dialog("QR code", $defs, true, 1150, $attrs);
        } catch (Exception $ex) {
        }

        return Action_Factory::status(0);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return $this->do_get_control_defs();
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        switch ($user_input->control_id) {

            case self::SETUP_ACTION_DONATE_DLG:
                return $this->do_donate_dialog();

            case self::CONTROL_STREAMING_SCREEN: // show streaming settings dialog
                return Action_Factory::open_folder(Starnet_Streaming_Setup_Screen::get_media_url_str(), TR::t('setup_streaming_settings'));

            case self::CONTROL_INTERFACE_SCREEN: // show interface settings dialog
                return Action_Factory::open_folder(Starnet_Interface_Setup_Screen::get_media_url_str(), TR::t('setup_interface_title'));

            case self::CONTROL_CHANNELS_SCREEN: // show epg settings dialog
                return Action_Factory::open_folder(Starnet_Channels_Setup_Screen::get_media_url_str(), TR::t('tv_screen_channels_setup'));

            case self::CONTROL_EPG_SCREEN: // show epg settings dialog
                return Action_Factory::open_folder(Starnet_Epg_Setup_Screen::get_media_url_str(), TR::t('setup_epg_settings'));

            case self::CONTROL_EXT_SETUP_SCREEN:
                return Action_Factory::open_folder(Starnet_Ext_Setup_Screen::get_media_url_str(), TR::t('setup_extended_setup'));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs());
    }
}

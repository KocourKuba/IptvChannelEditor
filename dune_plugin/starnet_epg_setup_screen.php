<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';
require_once 'lib/epg_manager.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Epg_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'epg_setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_EPG_SOURCE = 'epg_source';
    const SETUP_ACTION_XMLTV_EPG_IDX = 'xmltv_epg_idx';
    const SETUP_ACTION_CUSTOM_XMLTV_EPG = 'custom_xmltv_epg';
    const SETUP_ACTION_CHANGE_XMLTV_CACHE_PATH = 'xmltv_cache_path';
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
            $all_sources = $this->plugin->config->epg_man->get_xmltv_urls();
            if (empty($all_sources)) {
                Control_Factory::add_label($defs, 'no xmltv sources', '');
            } else {
                $xmltv_idx = $this->plugin->config->epg_man->get_xmltv_idx($plugin_cookies);
                hd_print(__METHOD__ . ": stored idx $xmltv_idx");
                if (!isset($all_sources[$xmltv_idx])) {
                    $xmltv_idx = array_search($all_sources, reset($all_sources));
                    hd_print(__METHOD__ . ": selected idx $xmltv_idx");
                }

                Control_Factory::add_combobox($defs, $this, null,
                    self::SETUP_ACTION_XMLTV_EPG_IDX, TR::t('setup_xmltv_epg_source'),
                    $xmltv_idx, $all_sources, self::CONTROLS_WIDTH, true);

                $xcache_dir = Epg_Manager::get_xcache_dir($plugin_cookies);
                $free_size = TR::t('setup_storage_info__1', HD::get_storage_size(dirname($xcache_dir)));
                if (is_apk()) {
                    Control_Factory::add_label($defs, $free_size, $xcache_dir);
                } else {
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANGE_XMLTV_CACHE_PATH,
                        $free_size, $xcache_dir, $this->plugin->get_image_path('folder.png'));
                }
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
                    return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);
                }
                break;

            case self::SETUP_ACTION_XMLTV_EPG_IDX:
                $val = $user_input->{self::SETUP_ACTION_XMLTV_EPG_IDX};
                $plugin_cookies->{self::SETUP_ACTION_XMLTV_EPG_IDX} = $val;
                hd_print(__METHOD__ . ": Selected xmltv epg idx: $val");
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case self::SETUP_ACTION_CHANGE_XMLTV_CACHE_PATH:
                $media_url = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'parent_id' => self::ID,
                        'allow_network' => false,
                        'save_data' => self::ID,
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url, TR::t('setup_epg_xmltv_cache_caption'));

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
                $this->plugin->config->epg_man->clear_epg_cache($plugin_cookies);
                return Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'));

            case ACTION_RESET_DEFAULT:
                hd_print(__METHOD__ . ": " . ACTION_RESET_DEFAULT);
                Epg_Manager::set_xcache_dir($plugin_cookies, MediaURL::make(array('filepath' => get_data_path())));
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', Epg_Manager::get_xcache_dir($plugin_cookies)),
                    User_Input_Handler_Registry::create_action($this, ACTION_RELOAD));

            case ACTION_FOLDER_SELECTED:
                $data = MediaURL::decode($user_input->selected_data);
                hd_print(__METHOD__ . ": " . ACTION_FOLDER_SELECTED . " $data->filepath");
                Epg_Manager::set_xcache_dir($plugin_cookies, $data);
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $data->caption),
                    User_Input_Handler_Registry::create_action($this, ACTION_RELOAD), $data->filepath, 800);

            case ACTION_RELOAD:
                hd_print(__METHOD__ . ": " . ACTION_RELOAD);
                $this->plugin->config->epg_man->clear_epg_memory_cache();
                $xmltv_idx = $plugin_cookies->{self::SETUP_ACTION_XMLTV_EPG_IDX};
                $cached_file = $this->plugin->config->epg_man->get_xml_cached_file($xmltv_idx, $plugin_cookies);
                $max_cache_time = 3600 * 24 * $plugin_cookies->epg_cache_ttl;

                hd_print(__METHOD__ . ": Checking: $cached_file ($xmltv_idx)");

                if (false === Epg_Manager::is_xmltv_index_valid($cached_file, $max_cache_time)) {
                    $url = $this->plugin->config->epg_man->get_xmltv_url($xmltv_idx);
                    $res = Epg_Manager::download_xmltv_url($url, $cached_file);
                    if (true !== $res) {
                        return Action_Factory::show_title_dialog(TR::t('err_load_xmltv_epg'),
                            Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies)),
                            $res, 800);
                    }
                }

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

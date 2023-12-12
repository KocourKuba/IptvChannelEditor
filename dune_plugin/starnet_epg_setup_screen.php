<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';
require_once 'lib/epg_manager.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Epg_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'epg_setup';

    const ACTION_RELOAD_EPG = 'reload_epg';
    const ACTION_CLEAR_EPG_CACHE = 'clear_epg_cache';
    const ACTION_EPG_SHIFT = 'epg_shift';
    const CONTROL_CHANGE_CACHE_PATH = 'xmltv_cache_path';
    const CONTROL_EPG_FOLDER = 'epg_folder';

    ///////////////////////////////////////////////////////////////////////

    /**
     * EPG dialog defs
     * @return array
     */
    public function do_get_control_defs()
    {
        $defs = array();

        $remove_icon = get_image_path('brush.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // EPG Source
        $epg_source_ops = array();
        $epg_source_ops[ENGINE_JSON] = TR::t('setup_epg_engine_json');
        $epg_source_ops[ENGINE_XMLTV] = TR::t('setup_epg_engine_xmltv');

        $cache_engine = $this->plugin->get_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_JSON);

        Control_Factory::add_combobox($defs, $this, null,
            PARAM_EPG_CACHE_ENGINE, TR::t('setup_epg_source'),
            $cache_engine, $epg_source_ops, self::CONTROLS_WIDTH, true);

        if ($cache_engine === ENGINE_XMLTV) {
            //////////////////////////////////////
            // Fuzzy search
            $fuzzy_search = $this->plugin->get_parameter(PARAM_FUZZY_SEARCH_EPG, SetupControlSwitchDefs::switch_off);
            Control_Factory::add_image_button($defs, $this, null,
                PARAM_FUZZY_SEARCH_EPG, TR::t('entry_epg_fuzzy_search'), SetupControlSwitchDefs::$on_off_translated[$fuzzy_search],
                get_image_path(SetupControlSwitchDefs::$on_off_img[$fuzzy_search]), self::CONTROLS_WIDTH);
        }

        //////////////////////////////////////
        // Fake EPG
        $fake_epg = $this->plugin->get_parameter(PARAM_FAKE_EPG, SetupControlSwitchDefs::switch_off);
        Control_Factory::add_image_button($defs, $this, null,
            PARAM_FAKE_EPG, TR::t('entry_epg_fake'), SetupControlSwitchDefs::$on_off_translated[$fake_epg],
            get_image_path(SetupControlSwitchDefs::$on_off_img[$fake_epg]), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // EPG cache dir
        $xcache_dir = $this->plugin->get_cache_dir();
        $free_size = TR::t('setup_storage_info__1', HD::get_storage_size($xcache_dir));
        $xcache_dir = HD::string_ellipsis($xcache_dir);
        Control_Factory::add_image_button($defs, $this, null, self::CONTROL_CHANGE_CACHE_PATH,
            $free_size, $xcache_dir, get_image_path('folder.png'), self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // EPG cache
        $epg_cache_ops = array();
        $epg_cache_ops[0.5] = 0.5;
        $epg_cache_ops[1] = 1;
        $epg_cache_ops[2] = 2;
        $epg_cache_ops[3] = 3;
        $epg_cache_ops[4] = 4;
        $epg_cache_ops[5] = 5;
        $epg_cache_ops[6] = 6;
        $epg_cache_ops[7] = 7;

        $cache_ttl = $this->plugin->get_parameter(PARAM_EPG_CACHE_TTL, 3);
        Control_Factory::add_combobox($defs, $this, null,
            PARAM_EPG_CACHE_TTL, TR::t('setup_epg_cache_ttl'),
            $cache_ttl, $epg_cache_ops, self::CONTROLS_WIDTH, true);

        //////////////////////////////////////
        // clear epg cache
        Control_Factory::add_image_button($defs, $this, null,
            self::ACTION_CLEAR_EPG_CACHE, TR::t('entry_epg_cache_clear'), TR::t('clear'),
            $remove_icon, self::CONTROLS_WIDTH);

        //////////////////////////////////////
        // epg time shift
        $show_epg_shift_ops = array();
        for ($i = -11; $i < 12; $i++) {
            $show_epg_shift_ops[$i] = TR::t('setup_epg_shift__1', sprintf("%+03d", $i));
        }
        $show_epg_shift_ops[0] = TR::t('setup_epg_shift_default__1', "00");

        $epg_shift = $this->plugin->get_parameter(PARAM_EPG_SHIFT, 0);
        Control_Factory::add_combobox($defs, $this, null,
            PARAM_EPG_SHIFT, TR::t('setup_epg_shift'),
            $epg_shift, $show_epg_shift_ops, self::CONTROLS_WIDTH, true);

        return $defs;
    }

    /**
     * @inheritDoc
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

        $action_reload = User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);
        $control_id = $user_input->control_id;
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            hd_debug_print("Setup: changing $control_id value to $new_value");
        }

        switch ($control_id) {
            case PARAM_EPG_CACHE_ENGINE:
                $this->plugin->tv->unload_channels();
                $this->plugin->set_parameter(PARAM_EPG_CACHE_ENGINE, $user_input->{$control_id});
                $this->plugin->init_epg_manager();
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case self::CONTROL_CHANGE_CACHE_PATH:
                $media_url_str = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'source_window_id' => static::ID,
                        'choose_folder' => array(
                            'action' => self::CONTROL_EPG_FOLDER,
                        ),
                        'allow_reset' => true,
                        'allow_network' => !is_apk(),
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url_str, TR::t('setup_epg_xmltv_cache_caption'));

            case PARAM_EPG_CACHE_TTL:
            case PARAM_EPG_SHIFT:
                $this->plugin->set_parameter($control_id, $user_input->{$control_id});
                hd_debug_print("$control_id: " . $user_input->{$control_id}, true);
                break;

            case self::ACTION_CLEAR_EPG_CACHE:
                if ($this->plugin->get_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_JSON) !== ENGINE_JSON) {
                    $this->plugin->tv->unload_channels();
                }
                $this->plugin->get_epg_manager()->clear_epg_cache();
                return Action_Factory::show_title_dialog(TR::t('entry_epg_cache_cleared'),
                    Action_Factory::reset_controls($this->do_get_control_defs()));

            case ACTION_RESET_DEFAULT:
                hd_debug_print(ACTION_RESET_DEFAULT);
                $this->plugin->get_epg_manager()->clear_all_epg_cache();
                $this->plugin->remove_parameter(PARAM_CACHE_PATH);
                $this->plugin->init_epg_manager();

                $default_path = $this->plugin->get_cache_dir();
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $default_path),
                    $action_reload, $default_path, self::CONTROLS_WIDTH);

            case self::CONTROL_EPG_FOLDER:
                $data = MediaURL::decode($user_input->selected_data);
                hd_debug_print(self::CONTROL_EPG_FOLDER . ": $data->filepath");
                if ($this->plugin->get_cache_dir() === $data->filepath) break;

                $this->plugin->get_epg_manager()->clear_all_epg_cache();
                $this->plugin->set_parameter(PARAM_CACHE_PATH, $data->filepath);
                $this->plugin->init_epg_manager();

                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $data->caption),
                    $action_reload, $data->filepath, self::CONTROLS_WIDTH);

            case PARAM_FUZZY_SEARCH_EPG:
            case PARAM_FAKE_EPG:
                $this->plugin->toggle_parameter($control_id, false);
                $this->plugin->init_epg_manager();
                break;

            case self::ACTION_RELOAD_EPG:
                hd_debug_print(self::ACTION_RELOAD_EPG);
                $this->plugin->get_epg_manager()->clear_epg_cache();
                $this->plugin->init_epg_manager();
                $res = $this->plugin->get_epg_manager()->is_xmltv_cache_valid();
                if ($res === -1) {
                    return Action_Factory::show_title_dialog(TR::t('err_epg_not_set'), null, HD::get_last_error());
                }

                if ($res === 0) {
                    $res = $this->plugin->get_epg_manager()->download_xmltv_source();
                    if ($res === -1) {
                        return Action_Factory::show_title_dialog(TR::t('err_load_xmltv_epg'), null, HD::get_last_error());
                    }
                }
                return $action_reload;

            case ACTION_RELOAD:
                hd_debug_print(ACTION_RELOAD);
                $this->plugin->tv->reload_channels();
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Action_Factory::invalidate_all_folders(Action_Factory::reset_controls($this->do_get_control_defs()));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs());
    }
}

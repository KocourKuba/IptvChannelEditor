<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Channels_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'channels_setup';

    const SETUP_ACTION_CHANGE_CH_LIST_PATH = 'change_list_path';
    const SETUP_ACTION_CHANNELS_URL_PATH = 'channels_url_path';
    const SETUP_ACTION_CHANNELS_URL_DLG = 'channels_url_dialog';
    const SETUP_ACTION_CHANNELS_URL_APPLY = 'channels_url_apply';
    const CONTROL_CHANNELS_FOLDER = 'channels_folder';

    ///////////////////////////////////////////////////////////////////////

    /**
     * defs for all controls on screen
     * @return array
     */
    public function do_get_control_defs()
    {
        hd_debug_print(null, true);
        $defs = array();

        $folder_icon = get_image_path('folder.png');
        $web_icon = get_image_path('web.png');
        $link_icon = get_image_path('link.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // channels list source
        $source_ops[1] = TR::t('setup_channels_src_folder');
        $source_ops[2] = TR::t('setup_channels_src_internet');
        $source_ops[3] = TR::t('setup_channels_src_direct');
        $channels_source = $this->plugin->get_parameter(PARAM_CHANNELS_SOURCE, 1);
        Control_Factory::add_combobox($defs, $this, null, PARAM_CHANNELS_SOURCE,
            TR::t('setup_channels_src_combo'), $channels_source, $source_ops, self::CONTROLS_WIDTH, true);

        switch ($channels_source)
        {
            case 1: // channels path
                $channels_list = smb_tree::get_folder_info($this->plugin->get_parameter(PARAM_CHANNELS_LIST_PATH, get_install_path()));

                $max_size = is_apk() ? 45 : 36;
                if (strlen($channels_list) > $max_size) {
                    $display_path = "..." . substr($channels_list, strlen($channels_list) - $max_size);
                } else {
                    $display_path = $channels_list;
                }

                if (is_apk()) {
                    Control_Factory::add_label($defs, TR::t('setup_channels_src_label'), $display_path);
                } else {
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANGE_CH_LIST_PATH,
                        TR::t('setup_channels_src_folder_path'), $display_path, $folder_icon);
                }
                break;
            case 2: // internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    TR::t('setup_channels_src_internet_path'), TR::t('setup_channels_src_change_caption'), $web_icon, self::CONTROLS_WIDTH);
                break;
            case 3: // direct internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    TR::t('setup_channels_src_direct_link'), TR::t('setup_channels_src_change_caption'), $link_icon, self::CONTROLS_WIDTH);
                break;
        }

        //////////////////////////////////////
        // channels lists
        $all_channels = $this->plugin->config->get_channel_list($channels_list);
        if (empty($all_channels)) {
            Control_Factory::add_label($defs, TR::t('setup_channels_src_used_label'), TR::t('setup_channels_src_no_channels'));
        } else {
            Control_Factory::add_combobox($defs, $this, null, PARAM_CHANNELS_LIST_NAME,
                TR::t('setup_channels_src_used_label'), $channels_list, $all_channels, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // playlist source
        $all_tv_lists = $this->plugin->config->get_tv_list_names($play_list_idx);
        hd_debug_print("current playlist index: $play_list_idx");

        if (count($all_tv_lists) > 1) {
            Control_Factory::add_combobox($defs, $this, null, ACTION_CHANGE_PLAYLIST,
                TR::t('setup_channels_src_playlist'), $play_list_idx,
                $all_tv_lists, self::CONTROLS_WIDTH, true);
        }

        Control_Factory::add_vgap($defs, 10);

        return $defs;
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
     * channels list url dialog defs
     * @return array
     */
    public function do_get_channels_url_control_defs()
    {
        $defs = array();

        $this->plugin->config->get_channel_list($channels_list);
        $source = $this->plugin->get_parameter(PARAM_CHANNELS_SOURCE, 1);
        $url_path = '';
        switch ($source) {
            case 2:
                $url_path = $this->plugin->config->plugin_info['app_channels_url_path'];
                break;
            case 3:
                if (isset($this->plugin->config->plugin_info['app_direct_links'][$channels_list])) {
                    $url_path = $this->plugin->config->plugin_info['app_direct_links'][$channels_list];
                }
                break;
            default:
                break;
        }

        Control_Factory::add_vgap($defs, 20);
        Control_Factory::add_text_field($defs, $this, null, 'channels_url_path', '',
            $url_path, false, false, false, true, self::CONTROLS_WIDTH);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null,
            self::SETUP_ACTION_CHANNELS_URL_APPLY, TR::t('ok'), 300);
        Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
        Control_Factory::add_vgap($defs, 10);

        return $defs;
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        $control_id = $user_input->control_id;
        $new_value = '';
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
        }

        switch ($control_id) {

            case self::SETUP_ACTION_CHANGE_CH_LIST_PATH:
                $media_url_str = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'source_window_id' => static::ID,
                        'choose_folder' => array(
                            'action' => self::CONTROL_CHANNELS_FOLDER,
                        ),
                        'allow_reset' => true,
                        'allow_network' => !is_apk(),
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url_str, TR::t('setup_channels_src_folder_caption'));

            case ACTION_CHANGE_PLAYLIST:
            case PARAM_CHANNELS_SOURCE:
                hd_debug_print("$control_id: " . $new_value);
                $this->plugin->set_parameter($control_id, $new_value);
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case PARAM_CHANNELS_LIST_NAME:
                hd_debug_print("$control_id: " . $new_value);
                $this->plugin->set_parameter($control_id, $new_value);
                $channel_list = empty($new_value) ? 'default' : $new_value;
                $favorites = 'favorite_channels_' . hash('crc32', $channel_list);

                if (isset($plugin_cookies->{$favorites})) {
                    $ids = array_filter(explode(",", $plugin_cookies->{$favorites}));
                    HD::put_data_items($favorites, $ids);
                    unset($plugin_cookies->{$favorites});
                }
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case self::SETUP_ACTION_CHANNELS_URL_DLG:
                $defs = $this->do_get_channels_url_control_defs();
                return Action_Factory::show_dialog(TR::t('setup_channels_src_link_caption'), $defs, true);

            case self::SETUP_ACTION_CHANNELS_URL_APPLY: // handle streaming settings dialog result
                if (!isset($user_input->{self::SETUP_ACTION_CHANNELS_URL_PATH})) break;

                $url_path = $user_input->{self::SETUP_ACTION_CHANNELS_URL_PATH};
                $source = $this->plugin->get_parameter(PARAM_CHANNELS_SOURCE, 1);
                switch ($source) {
                    case 2:
                    case 3:
                        if ($url_path !== $this->plugin->get_parameter($source === 2 ? PARAM_CHANNELS_URL : PARAM_CHANNELS_DIRECT_URL)) {
                            $this->plugin->set_parameter($source === 2 ? PARAM_CHANNELS_URL : PARAM_CHANNELS_DIRECT_URL, $url_path);
                        }
                        break;
                }

                hd_debug_print("Selected channels path: $url_path");
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case self::CONTROL_CHANNELS_FOLDER:
                $data = MediaURL::decode($user_input->selected_data);
                hd_debug_print(self::CONTROL_CHANNELS_FOLDER . " $data->filepath");
                $this->plugin->set_parameter(PARAM_CHANNELS_LIST_PATH, smb_tree::set_folder_info($user_input->selected_data));
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $data->caption),
                    User_Input_Handler_Registry::create_action($this, ACTION_RELOAD), $data->filepath, self::CONTROLS_WIDTH);

            case ACTION_RESET_DEFAULT:
                hd_debug_print(ACTION_RESET_DEFAULT);
                $this->plugin->remove_parameter(PARAM_CHANNELS_LIST_PATH);
                return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);

            case ACTION_RELOAD:
                hd_debug_print(ACTION_RELOAD);
                $res = $this->plugin->tv->reload_channels();
                if ($res === -1) {
                    return Action_Factory::show_title_dialog(TR::t('err_load_channels_list'));
                }

                return Action_Factory::invalidate_all_folders($plugin_cookies,
                    Action_Factory::reset_controls($this->do_get_control_defs()));
        }

        return Action_Factory::reset_controls($this->do_get_control_defs());
    }
}

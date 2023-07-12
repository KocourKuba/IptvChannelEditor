<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Channels_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'channels_setup';

    const SETUP_ACTION_CHANGE_CH_LIST_PATH = 'change_list_path';
    const SETUP_ACTION_CHANGE_CH_LIST = 'change_channels_list';
    const SETUP_ACTION_CHANNELS_SOURCE = 'channels_source';
    const SETUP_ACTION_CHANNELS_URL_DLG = 'channels_url_dialog';
    const SETUP_ACTION_CHANNELS_URL_APPLY = 'channels_url_apply';
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
     * defs for all controls on screen
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_control_defs(&$plugin_cookies)
    {
        hd_print(__METHOD__);
        $defs = array();

        $folder_icon = $this->plugin->get_image_path('folder.png');
        $web_icon = $this->plugin->get_image_path('web.png');
        $link_icon = $this->plugin->get_image_path('link.png');
        $refresh_icon = $this->plugin->get_image_path('refresh.png');

        //////////////////////////////////////
        // Plugin name
        $this->plugin->create_setup_header($defs);

        //////////////////////////////////////
        // channels list source
        $source_ops[1] = TR::t('setup_channels_src_folder');
        $source_ops[2] = TR::t('setup_channels_src_internet');
        $source_ops[3] = TR::t('setup_channels_src_direct');
        $channels_source = isset($plugin_cookies->channels_source) ? (int)$plugin_cookies->channels_source : 1;

        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANNELS_SOURCE,
            TR::t('setup_channels_src_combo'), $channels_source, $source_ops, self::CONTROLS_WIDTH, true);

        switch ($channels_source)
        {
            case 1: // channels path
                $display_path = smb_tree::get_folder_info($plugin_cookies, PARAM_CH_LIST_PATH, get_install_path());

                if (is_apk()) {
                    Control_Factory::add_label($defs, TR::t('setup_channels_src_label'), $display_path);
                } else {
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANGE_CH_LIST_PATH,
                        TR::t('setup_channels_src_folder_path'), $display_path, $folder_icon);

                    if ($display_path !== get_install_path()) {
                        Control_Factory::add_image_button($defs, $this, null, ACTION_RESET_DEFAULT,
                            TR::t('reset_default'), TR::t('apply'), $refresh_icon);
                    } else {
                        Control_Factory::add_label($defs, TR::t('reset_default'), TR::t('apply'));
                    }
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
        $all_channels = $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
        if (empty($all_channels)) {
            Control_Factory::add_label($defs, TR::t('setup_channels_src_used_label'), TR::t('setup_channels_src_no_channels'));
        } else {
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANGE_CH_LIST,
                TR::t('setup_channels_src_used_label'), $channels_list, $all_channels, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // playlist source
        $all_tv_lists = $this->plugin->config->get_tv_list_names($plugin_cookies, $play_list_idx);
        hd_print(__METHOD__ . ": current playlist index: $play_list_idx");

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
        return $this->do_get_control_defs($plugin_cookies);
    }

    /**
     * channels list url dialog defs
     * @param $plugin_cookies
     * @return array
     */
    public function do_get_channels_url_control_defs(&$plugin_cookies)
    {
        hd_print(__METHOD__);
        $defs = array();

        $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
        $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;
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
     * user remote input handler Implementation of UserInputHandler
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        $control_id = $user_input->control_id;
        $new_value = '';
        if (isset($user_input->action_type, $user_input->{$control_id})
            && ($user_input->action_type === 'confirm' || $user_input->action_type === 'apply')) {
            $new_value = $user_input->{$control_id};
            //hd_print(__METHOD__ . ": Setup: changing $control_id value to $new_value");
        }

        switch ($control_id) {

            case ACTION_CHANGE_PLAYLIST:
                $old_value = $plugin_cookies->playlist_idx;
                $plugin_cookies->playlist_idx = $new_value;
                hd_print(__METHOD__ . ": current playlist idx: $new_value");
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    $plugin_cookies->playlist_idx = $old_value;
                    Action_Factory::show_title_dialog(TR::t('err_load_playlist'));
                }
                return $action;

            case self::SETUP_ACTION_CHANGE_CH_LIST_PATH:
                $media_url = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'parent_id' => self::ID,
                        'save_data' => self::ID,
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url, TR::t('setup_channels_src_folder_caption'));

            case self::SETUP_ACTION_CHANGE_CH_LIST:
                $old_value = $plugin_cookies->channels_list;
                $plugin_cookies->channels_list = $new_value;
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    $plugin_cookies->channels_list = $old_value;
                    Action_Factory::show_title_dialog(TR::t('err_load_channels_list'));
                }
                return $action;

            case self::SETUP_ACTION_CHANNELS_SOURCE: // handle streaming settings dialog result
                $plugin_cookies->channels_source = $user_input->channels_source;
                hd_print(__METHOD__ . ": Selected channels source: $plugin_cookies->channels_source");
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    Action_Factory::show_title_dialog(TR::t('err_load_channels_list'));
                }
                return $action;

            case self::SETUP_ACTION_CHANNELS_URL_DLG:
                $defs = $this->do_get_channels_url_control_defs($plugin_cookies);
                return Action_Factory::show_dialog(TR::t('setup_channels_src_link_caption'), $defs, true);

            case self::SETUP_ACTION_CHANNELS_URL_APPLY: // handle streaming settings dialog result
                if (isset($user_input->channels_url_path)) {
                    $source = isset($plugin_cookies->channels_source) ? $plugin_cookies->channels_source : 1;

                    switch ($source) {
                        case 2:
                            if ($user_input->channels_url_path !== $plugin_cookies->channels_url) {
                                $plugin_cookies->channels_url = $user_input->channels_url_path;
                            }
                            break;
                        case 3:
                            if ($user_input->channels_url_path !== $plugin_cookies->channels_direct_url) {
                                $plugin_cookies->channels_direct_url = $user_input->channels_url_path;
                            }
                            break;
                    }

                    hd_print(__METHOD__ . ": Selected channels path: $plugin_cookies->channels_url");
                }

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

            case ACTION_FOLDER_SELECTED:
                $data = MediaURL::decode($user_input->selected_data);
                hd_print(__METHOD__ . ": " . ACTION_FOLDER_SELECTED . " $data->filepath");
                smb_tree::set_folder_info($plugin_cookies, $data, PARAM_CH_LIST_PATH);
                return Action_Factory::show_title_dialog(TR::t('folder_screen_selected_folder__1', $data->caption),
                    User_Input_Handler_Registry::create_action($this, ACTION_RELOAD), $data->filepath, self::CONTROLS_WIDTH);

            case ACTION_RELOAD:
                hd_print(__METHOD__ . ": reload");
                return $this->plugin->tv->reload_channels($this, $plugin_cookies);

            case ACTION_RESET_DEFAULT:
                //hd_print(__METHOD__ . ": reset_folder");
                $plugin_cookies->ch_list_path = '';
                return $this->plugin->tv->reload_channels($this, $plugin_cookies);
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

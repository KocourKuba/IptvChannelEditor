<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Channels_Setup_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'channels_setup';
    const CONTROLS_WIDTH = 800;

    const SETUP_ACTION_CHANGE_CH_LIST_PATH = 'change_list_path';
    const SETUP_ACTION_CHANGE_CH_LIST = 'change_channels_list';
    const SETUP_ACTION_CHANGE_PL_LIST = 'change_playlist';
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

        //////////////////////////////////////
        // Plugin name
        Control_Factory::add_vgap($defs, -10);
        $title = " v.{$this->plugin->config->plugin_info['app_version']} [{$this->plugin->config->plugin_info['app_release_date']}]";
        Control_Factory::add_label($defs, "IPTV Channel Editor by sharky72", $title, 20);

        //////////////////////////////////////
        // channels list source
        $source_ops[1] = 'Локальная или сетевая папки';
        $source_ops[2] = 'Интернет/Интранет папка';
        $source_ops[3] = 'Прямая Интернет ссылка';
        $channels_source = isset($plugin_cookies->channels_source) ? (int)$plugin_cookies->channels_source : 1;

        Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANNELS_SOURCE,
            'Источник списка каналов:', $channels_source, $source_ops, self::CONTROLS_WIDTH, true);

        switch ($channels_source)
        {
            case 1: // channels path
                $display_path = smb_tree::get_folder_info($plugin_cookies, 'ch_list_path');
                if (strlen($display_path) > 30) {
                    $display_path = "..." . substr($display_path, -30);
                }
                if (is_apk())
                    Control_Factory::add_label($defs, 'Папка со списками каналов:', $display_path);
                else
                    Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANGE_CH_LIST_PATH,
                        'Задать папку со списками каналов:', $display_path, $folder_icon);
                break;
            case 2: // internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    'Задать ссылку со списками каналов:', 'Изменить ссылку', $web_icon, self::CONTROLS_WIDTH);
                break;
            case 3: // direct internet url
                Control_Factory::add_image_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_DLG,
                    'Задать ссылку на список каналов:', 'Изменить ссылку', $link_icon, self::CONTROLS_WIDTH);
                break;
        }

        //////////////////////////////////////
        // channels lists
        $all_channels = $this->plugin->config->get_channel_list($plugin_cookies, $channels_list);
        if (empty($all_channels)) {
            Control_Factory::add_label($defs, 'Используемый список каналов:', 'Нет списка каналов!!!');
        } else {
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANGE_CH_LIST,
                'Используемый список каналов:', $channels_list, $all_channels, self::CONTROLS_WIDTH, true);
        }

        //////////////////////////////////////
        // playlist source
        $all_tv_lists = $this->plugin->config->get_tv_list_names($plugin_cookies, $play_list_idx);
        hd_print("current playlist index: $play_list_idx");

        if (count($all_tv_lists) > 1) {
            Control_Factory::add_combobox($defs, $this, null, self::SETUP_ACTION_CHANGE_PL_LIST,
                'Источник плейлистов:', $play_list_idx,
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
            $url_path, false, false, false, true, 800);

        Control_Factory::add_vgap($defs, 50);

        Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, self::SETUP_ACTION_CHANNELS_URL_APPLY, 'ОК', 300);
        Control_Factory::add_close_dialog_button($defs, 'Отмена', 300);
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
            hd_print("Setup: changing $control_id value to $new_value");
        }

        switch ($control_id) {

            case self::SETUP_ACTION_CHANGE_PL_LIST:
                $old_value = $plugin_cookies->playlist_idx;
                $plugin_cookies->playlist_idx = $new_value;
                hd_print("current playlist idx: $new_value");
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    $plugin_cookies->playlist_idx = $old_value;
                    Action_Factory::show_title_dialog("Ошибка загрузки плейлиста!");
                }
                return $action;

            case self::SETUP_ACTION_CHANGE_CH_LIST_PATH:
                $media_url = MediaURL::encode(
                    array(
                        'screen_id' => Starnet_Folder_Screen::ID,
                        'save_data' => 'channels_list_path',
                        'windowCounter' => 1,
                    )
                );
                return Action_Factory::open_folder($media_url, 'Папка со списком каналов');

            case self::SETUP_ACTION_CHANGE_CH_LIST:
                $old_value = $plugin_cookies->channels_list;
                $plugin_cookies->channels_list = $new_value;
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    $plugin_cookies->channels_list = $old_value;
                    Action_Factory::show_title_dialog("Ошибка загрузки списка каналов!");
                }
                return $action;

            case self::SETUP_ACTION_CHANNELS_SOURCE: // handle streaming settings dialog result
                $plugin_cookies->channels_source = $user_input->channels_source;
                hd_print("Selected channels source: $plugin_cookies->channels_source");
                $action = $this->plugin->tv->reload_channels($this, $plugin_cookies);
                if ($action === null) {
                    Action_Factory::show_title_dialog("Ошибка загрузки списка каналов!");
                }
                return $action;

            case self::SETUP_ACTION_CHANNELS_URL_DLG:
                $defs = $this->do_get_channels_url_control_defs($plugin_cookies);
                return Action_Factory::show_dialog('Ссылка на списки каналов', $defs, true);

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

                    hd_print("Selected channels path: $plugin_cookies->channels_url");
                }

                return $this->plugin->tv->reload_channels($this, $plugin_cookies);
        }

        return Action_Factory::reset_controls($this->do_get_control_defs($plugin_cookies));
    }
}

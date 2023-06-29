<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_TV_History_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_history';

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_HISTORY_LIST_FOLDER_VIEWS());

        $plugin->create_screen($this);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $action_play = User_Input_Handler_Registry::create_action($this, ACTION_OPEN_FOLDER);
        return array(
            GUI_EVENT_KEY_ENTER    => $action_play,
            GUI_EVENT_KEY_PLAY     => $action_play,
            GUI_EVENT_KEY_B_GREEN  => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete')),
            GUI_EVENT_KEY_C_YELLOW => User_Input_Handler_Registry::create_action($this, ACTION_ITEMS_CLEAR, TR::t('clear_history')),
            GUI_EVENT_KEY_D_BLUE   => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
        );
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $channel_id = $media_url->channel_id;

        switch ($user_input->control_id)
		{
            case ACTION_OPEN_FOLDER:
                return Action_Factory::tv_play($media_url);

			case ACTION_ITEM_DELETE:
                Playback_Points::clear(smb_tree::get_folder_info($plugin_cookies, PARAM_HISTORY_PATH), $channel_id);
				$parent_media_url = MediaURL::decode($user_input->parent_media_url);
				$sel_ndx = $user_input->sel_ndx + 1;
				if ($sel_ndx < 0)
					$sel_ndx = 0;
				$range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders(null,
                    Action_Factory::update_regular_folder($range, true, $sel_ndx));

            case ACTION_ITEMS_CLEAR:
                Playback_Points::clear(smb_tree::get_folder_info($plugin_cookies, PARAM_HISTORY_PATH));
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders(null,
                    Action_Factory::update_regular_folder($range, true));

			case ACTION_ADD_FAV:
				$is_favorite = $this->plugin->tv->is_favorite_channel_id($channel_id, $plugin_cookies);
				$opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
				$message = $is_favorite ? TR::t('deleted_from_favorite') : TR::t('added_to_favorite');
				$this->plugin->tv->change_tv_favorites($opt_type, $channel_id, $plugin_cookies);
				return Action_Factory::show_title_dialog($message);
		}

        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print(__METHOD__ . ": get_all_folder_items");

        $items = array();
        $now = time();
        foreach (Playback_Points::get_all() as $channel_id => $channel_ts) {
            if (is_null($channel = $this->plugin->tv->get_channel($channel_id))) continue;

            $prog_info = $this->plugin->tv->get_program_info($channel_id, $channel_ts, $plugin_cookies);
            $description = '';
            if (is_null($prog_info)) {
                $title = $channel->get_title();
            } else {
                // program epg available
                $title = $prog_info[PluginTvEpgProgram::name];
                if ($channel_ts > 0) {
                    $start_tm = $prog_info[PluginTvEpgProgram::start_tm_sec];
                    $epg_len = $prog_info[PluginTvEpgProgram::end_tm_sec] - $start_tm;
                    $description = $prog_info[PluginTvEpgProgram::description];
                    if ($channel_ts >= $now - $channel->get_archive_past_sec() - 60) {
                        $progress = max(0.01, min(1.0, round(($channel_ts - $start_tm) / $epg_len, 2))) * 100;
                        $title = "$title | " . date("j.m H:i", $channel_ts) . " [$progress%]";
                    }
                }
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(
                    array(
                        'channel_id' => $channel_id,
                        'group_id' => Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID,
                        'archive_tm' => $channel_ts
                    )
                ),
                PluginRegularFolderItem::caption => $title,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $channel->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $channel->get_icon_url(),
                    ViewItemParams::item_detailed_info => $description,
                ),
                PluginRegularFolderItem::starred => false,
            );
        }

        return $items;
    }
}
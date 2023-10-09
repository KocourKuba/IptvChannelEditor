<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Tv_Favorites_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_favorites';

    /**
     * @param string $group_id
     * @return false|string
     */
    public static function get_media_url_string($group_id)
    {
        return MediaURL::encode(array('screen_id' => static::ID, 'group_id' => $group_id));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $action_play = User_Input_Handler_Registry::create_action($this, ACTION_PLAY_ITEM);

        return array
        (
            GUI_EVENT_KEY_ENTER      => $action_play,
            GUI_EVENT_KEY_PLAY       => $action_play,
            GUI_EVENT_KEY_B_GREEN    => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up')),
            GUI_EVENT_KEY_C_YELLOW   => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down')),
            GUI_EVENT_KEY_D_BLUE     => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete')),
            GUI_EVENT_KEY_POPUP_MENU => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_KEY_RETURN     => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_RETURN),
        );
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $sel_ndx = $user_input->sel_ndx;
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $selected_media_url = MediaURL::decode($user_input->selected_media_url);

        switch ($user_input->control_id) {
            case ACTION_PLAY_ITEM:
                try {
                    $post_action = $this->plugin->tv_player_exec($selected_media_url);
                } catch (Exception $ex) {
                    hd_debug_print("Channel can't played, exception info: " . $ex->getMessage());
                    return Action_Factory::show_title_dialog(TR::t('err_channel_cant_start'),
                        null,
                        TR::t('warn_msg2__1', $ex->getMessage()));
                }

                return $this->plugin->update_epfs_data($plugin_cookies, null, $post_action);

            case ACTION_ITEM_UP:
                $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_MOVE_UP, $selected_media_url->channel_id);
                $sel_ndx--;
                if ($sel_ndx < 0) {
                    $sel_ndx = 0;
                }
                break;

            case ACTION_ITEM_DOWN:
                $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_MOVE_DOWN, $selected_media_url->channel_id);
                $sel_ndx++;
                if ($sel_ndx >= $this->plugin->get_favorites()->size()) {
                    $sel_ndx = $this->plugin->get_favorites()->size() - 1;
                }
                break;

            case ACTION_ITEM_DELETE:
                $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_REMOVE, $selected_media_url->channel_id);
                break;

            case ACTION_ITEMS_CLEAR:
                $this->plugin->change_tv_favorites(ACTION_ITEMS_CLEAR, null);
                break;

            case GUI_EVENT_KEY_POPUP_MENU:
                $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_CLEAR, TR::t('clear_favorites'), "brush.png");
                return Action_Factory::show_popup_menu($menu_items);

            case GUI_EVENT_KEY_RETURN:
                return $this->plugin->update_epfs_data($plugin_cookies, null, Action_Factory::close_and_run());

            default:
                return null;
        }

        return $this->invalidate_current_folder($parent_media_url, $plugin_cookies, $sel_ndx);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url->get_media_url_str(), true);

        $items = array();

        foreach ($this->plugin->get_favorites() as $channel_id) {
            if (!preg_match('/\S/', $channel_id)) {
                continue;
            }

            $channel = $this->plugin->tv->get_channel($channel_id);
            if (is_null($channel)) {
                hd_debug_print("Unknown channel $channel_id");
                $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_REMOVE, $channel_id);
                continue;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array(
                        'channel_id' => $channel->get_id(),
                        'group_id' => FAVORITES_GROUP_ID)
                ),
                PluginRegularFolderItem::caption => $channel->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $channel->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $channel->get_icon_url(),
                ),
                PluginRegularFolderItem::starred => false,
            );
        }

        return $items;
    }

    /**
     * @param MediaURL $parent_media_url
     * @param $plugin_cookies
     * @param int $sel_ndx
     * @return array
     */
    public function invalidate_current_folder(MediaURL $parent_media_url, $plugin_cookies, $sel_ndx = -1)
    {
        hd_debug_print(null, true);

        return Starnet_Epfs_Handler::invalidate_folders(array(static::ID),
            Action_Factory::update_regular_folder(
                $this->get_folder_range($parent_media_url, 0, $plugin_cookies),
                true,
                $sel_ndx)
        );
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('icons_4x3_no_caption'),
            $this->plugin->get_screen_view('icons_4x3_caption'),
            $this->plugin->get_screen_view('icons_3x3_no_caption'),
            $this->plugin->get_screen_view('icons_3x3_caption'),
            $this->plugin->get_screen_view('icons_5x3_no_caption'),
            $this->plugin->get_screen_view('icons_5x3_caption'),
            $this->plugin->get_screen_view('icons_5x4_no_caption'),
            $this->plugin->get_screen_view('icons_5x4_caption'),

            $this->plugin->get_screen_view('list_1x11_info'),
            $this->plugin->get_screen_view('list_2x11_small_info'),
            $this->plugin->get_screen_view('list_3x11_no_info'),
        );
    }
}

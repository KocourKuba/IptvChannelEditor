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
            GUI_EVENT_KEY_B_GREEN    => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('left')),
            GUI_EVENT_KEY_C_YELLOW   => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('right')),
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

        $selected_media_url = MediaURL::decode($user_input->selected_media_url);

        switch ($user_input->control_id) {
            case ACTION_PLAY_ITEM:
                try {
                    $post_action = $this->plugin->tv_player_exec($selected_media_url);
                } catch (Exception $ex) {
                    hd_debug_print("Channel can't played");
                    print_backtrace_exception($ex);
                    return Action_Factory::show_title_dialog(TR::t('err_channel_cant_start'),
                        null,
                        TR::t('warn_msg2__1', $ex->getMessage()));
                }

                $this->plugin->save_favorites();
                return $this->plugin->invalidate_epfs_folders($plugin_cookies, null, $post_action);

            case ACTION_ITEM_UP:
                $user_input->sel_ndx--;
                if ($user_input->sel_ndx < 0) {
                    $user_input->sel_ndx = 0;
                }
                return $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_MOVE_UP, $selected_media_url->channel_id);

            case ACTION_ITEM_DOWN:
                $user_input->sel_ndx++;
                if ($user_input->sel_ndx >= $this->plugin->get_favorites()->size()) {
                    $user_input->sel_ndx = $this->plugin->get_favorites()->size() - 1;
                }
                return $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_MOVE_DOWN, $selected_media_url->channel_id);

            case ACTION_ITEM_DELETE:
                if ($this->plugin->get_channels_list_favorites()->in_order($selected_media_url->channel_id)) {
                    return Action_Factory::show_error(false, TR::t('err_delete_embedded_fav'));
                }

                $action = $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_REMOVE, $selected_media_url->channel_id);
                if ($this->plugin->get_favorites()->size() !== 0) {
                    return $action;
                }

                return User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_RETURN);

            case ACTION_ITEMS_CLEAR:
                $this->plugin->change_tv_favorites(ACTION_ITEMS_CLEAR, null);
                return User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_RETURN);

            case GUI_EVENT_KEY_POPUP_MENU:
                $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_CLEAR, TR::t('clear_favorites'), "brush.png");
                return Action_Factory::show_popup_menu($menu_items);

            case GUI_EVENT_KEY_RETURN:
                $this->plugin->save_favorites();
                return $this->plugin->invalidate_epfs_folders($plugin_cookies, null, Action_Factory::close_and_run(), true);
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url, true);

        $items = array();

        foreach ($this->plugin->get_favorites() as $channel_id) {
            $channel = $this->plugin->tv->get_channel($channel_id);
            if (is_null($channel)) {
                hd_debug_print("Unknown channel $channel_id");
                $this->plugin->change_tv_favorites(PLUGIN_FAVORITES_OP_REMOVE, $channel_id);
                continue;
            }

            $items[] = array(
                PluginRegularFolderItem::media_url => MediaURL::encode(array(
                        'channel_id' => $channel->get_id(),
                        'group_id' => FAVORITES_GROUP_ID)
                ),
                PluginRegularFolderItem::caption => $channel->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $channel->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $channel->get_icon_url(),
                ),
            );

            if (defined('PluginRegularFolderItem::locked')) {
                $items[][PluginRegularFolderItem::locked] = $this->plugin->get_channels_list_favorites()->in_order($channel_id);
            }
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

        return Starnet_Epfs_Handler::epfs_invalidate_folders(array(static::ID),
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

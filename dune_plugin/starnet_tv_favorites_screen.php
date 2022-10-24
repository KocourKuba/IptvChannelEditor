<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/tv/tv.php';

class Starnet_Tv_Favorites_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'tv_favorites';

    const ACTION_ITEM_UP = 'item_up';
    const ACTION_ITEM_DOWN = 'item_down';
    const ACTION_ITEM_DELETE = 'item_delete';
    const ACTION_ITEMS_CLEAR = 'items_clear';

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array(
                'screen_id' => self::ID,
                'is_favorites' => true)
        );
    }

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_TV_CHANNEL_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_feature(Plugin_Constants::TV_FAVORITES_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $move_backward_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_UP);
        $move_backward_favorite_action['caption'] = 'Вверх';

        $move_forward_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_DOWN);
        $move_forward_favorite_action['caption'] = 'Вниз';

        $remove_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_DELETE);
        $remove_favorite_action['caption'] = 'Удалить';

        $remove_all_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEMS_CLEAR);

        $menu_items = array(
            array(GuiMenuItemDef::caption => 'Удалить из Избранного', GuiMenuItemDef::action => $remove_favorite_action),
            array(GuiMenuItemDef::caption => 'Очистить Избранное', GuiMenuItemDef::action => $remove_all_favorite_action),
        );

        $popup_menu_action = Action_Factory::show_popup_menu($menu_items);

        return array
        (
            GUI_EVENT_KEY_SETUP => Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина'),
            GUI_EVENT_KEY_ENTER => Action_Factory::tv_play(),
            GUI_EVENT_KEY_PLAY => Action_Factory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => $move_backward_favorite_action,
            GUI_EVENT_KEY_C_YELLOW => $move_forward_favorite_action,
            GUI_EVENT_KEY_D_BLUE => $remove_favorite_action,
            GUI_EVENT_KEY_POPUP_MENU => $popup_menu_action,
        );
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param int $sel_increment
     * @param $user_input
     * @param &$plugin_cookies
     * @return array
     */
    private function get_update_action($sel_increment, $user_input, &$plugin_cookies)
    {
        $num_favorites = count($this->plugin->tv->get_fav_channel_ids($plugin_cookies));

        $sel_ndx = $user_input->sel_ndx + $sel_increment;
        if ($sel_ndx < 0) {
            $sel_ndx = 0;
        }
        if ($sel_ndx >= $num_favorites) {
            $sel_ndx = $num_favorites - 1;
        }

        $post_action = Action_Factory::close_and_run(
            Action_Factory::open_folder(
                $user_input->parent_media_url,
                null,
                null,
                null,
                Action_Factory::update_regular_folder(
                    HD::create_regular_folder_range($this->get_all_folder_items(MediaURL::decode($user_input->parent_media_url), $plugin_cookies)),
                    true,
                    $sel_ndx)
            )
        );

        if (NEWGUI_FEAUTURES_AVAILABLE)
        {
            Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);
            return Starnet_Epfs_Handler::invalidate_folders(array($user_input->parent_media_url), $post_action);
        }

        return Action_Factory::invalidate_folders(array(Starnet_Main_Screen::get_media_url_str()), $post_action);
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Tv favorites: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        switch ($user_input->control_id) {
            case self::ACTION_ITEM_UP:
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_UP;
                $inc = -1;
                break;
            case self::ACTION_ITEM_DOWN:
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_DOWN;
                $inc = 1;
                break;
            case self::ACTION_ITEM_DELETE:
                $fav_op_type = PLUGIN_FAVORITES_OP_REMOVE;
                $inc = 0;
                break;
            case self::ACTION_ITEMS_CLEAR:
                $fav_op_type = 'clear_favorites';
                $inc = 0;
                break;
            default:
                return null;
        }

        $channel_id = MediaURL::decode($user_input->selected_media_url)->channel_id;
        $this->plugin->tv->change_tv_favorites($fav_op_type, $channel_id, $plugin_cookies);
        return $this->get_update_action($inc, $user_input, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);

        $items = array();

        foreach ($fav_channel_ids as $channel_id) {
            if (!preg_match('/\S/', $channel_id)) {
                continue;
            }

            $channel = $this->plugin->tv->get_channel($channel_id);
            if (is_null($channel)) {
                hd_print("Unknown channel $channel_id");
                $this->plugin->tv->change_tv_favorites(PLUGIN_FAVORITES_OP_REMOVE, $channel_id, $plugin_cookies);
                continue;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array(
                        'channel_id' => $channel->get_id(),
                        'group_id' => '__favorites')
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
}

<?php

require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_Vod_Favorites_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_favorites';

    const ACTION_ITEM_UP = 'item_up';
    const ACTION_ITEM_DOWN = 'item_down';
    const ACTION_ITEM_DELETE = 'item_delete';
    const ACTION_ITEMS_CLEAR = 'items_clear';

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
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_list_folder_views());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
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
        $play_action = $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play();

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
            GUI_EVENT_KEY_ENTER => $play_action,
            GUI_EVENT_KEY_PLAY => $play_action,
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
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Vod favorites: handle_user_input:');
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

        $movie_id = MediaURL::decode($user_input->selected_media_url)->movie_id;
        $this->plugin->vod->change_vod_favorites($fav_op_type, $movie_id, $plugin_cookies);
        return $this->get_update_action($inc, $user_input, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param int $sel_increment
     * @param $user_input
     * @param &$plugin_cookies
     * @return array
     */
    private function get_update_action($sel_increment, $user_input, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);

        $num_favorites = count($this->plugin->vod->get_favorite_movie_ids());

        $sel_ndx = $user_input->sel_ndx + $sel_increment;
        if ($sel_ndx < 0) {
            $sel_ndx = 0;
        }
        if ($sel_ndx >= $num_favorites) {
            $sel_ndx = $num_favorites - 1;
        }

        $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));
        return Action_Factory::update_regular_folder($range, true, $sel_ndx);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);

        $movie_ids = $this->plugin->vod->get_favorite_movie_ids();

        $items = array();

        foreach ($movie_ids as $movie_id) {
            if (empty($movie_id)) continue;

            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);
            if (is_null($short_movie)) {
                $caption = "Информация о фильме недоступна";
                $poster_url = "missing://";
            } else {
                $caption = $short_movie->name;
                $poster_url = $short_movie->poster_url;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Movie_Screen::get_media_url_str($movie_id),
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $poster_url,
                )
            );
        }

        return $items;
    }

    /**
     * @param MediaURL $media_url
     * @return null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}

<?php

require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Favorites_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_favorites';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $play_action = $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play();

        $move_backward_favorite_action = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up'));
        $move_forward_favorite_action = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down'));
        $remove_favorite_action = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete'));
        $remove_all_favorite_action = User_Input_Handler_Registry::create_action($this, ACTION_ITEMS_CLEAR);

        $menu_items = array(
            array(GuiMenuItemDef::caption => 'Очистить Избранное', GuiMenuItemDef::action => $remove_all_favorite_action),
        );

        $popup_menu_action = Action_Factory::show_popup_menu($menu_items);

        return array
        (
            GUI_EVENT_KEY_ENTER      => $play_action,
            GUI_EVENT_KEY_PLAY       => $play_action,
            GUI_EVENT_KEY_B_GREEN    => $move_backward_favorite_action,
            GUI_EVENT_KEY_C_YELLOW   => $move_forward_favorite_action,
            GUI_EVENT_KEY_D_BLUE     => $remove_favorite_action,
            GUI_EVENT_KEY_POPUP_MENU => $popup_menu_action,
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

        switch ($user_input->control_id) {
            case ACTION_ITEM_UP:
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_UP;
                $inc = -1;
                break;
            case ACTION_ITEM_DOWN:
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_DOWN;
                $inc = 1;
                break;
            case ACTION_ITEM_DELETE:
                $fav_op_type = PLUGIN_FAVORITES_OP_REMOVE;
                $inc = 0;
                break;
            case ACTION_ITEMS_CLEAR:
                $fav_op_type = ACTION_ITEMS_CLEAR;
                $inc = 0;
                break;
            default:
                return null;
        }

        $movie_id = MediaURL::decode($user_input->selected_media_url)->movie_id;
        $this->plugin->vod->change_vod_favorites($fav_op_type, $movie_id, $plugin_cookies);

        $num_favorites = count($this->plugin->vod->get_favorite_movie_ids());

        $sel_ndx = $user_input->sel_ndx + $inc;
        if ($sel_ndx < 0) {
            $sel_ndx = 0;
        }
        if ($sel_ndx >= $num_favorites) {
            $sel_ndx = $num_favorites - 1;
        }

        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $range = $this->create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));
        return Action_Factory::update_regular_folder($range, true, $sel_ndx);
    }

    ///////////////////////////////////////////////////////////////////////

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
            $this->plugin->vod->ensure_movie_loaded($movie_id, $plugin_cookies);
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);

            if (is_null($short_movie)) {
                $caption = TR::t('vod_screen_no_film_info');
                $poster_url = "missing://";
            } else {
                $caption = $short_movie->name;
                $poster_url = $short_movie->poster_url;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Movie_Screen::get_media_url_string($movie_id),
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
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('icons_5x2_movie_no_caption'),
            $this->plugin->get_screen_view('list_1x12_vod_info_normal'),
        );
    }
}

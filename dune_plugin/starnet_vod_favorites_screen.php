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
        return array(
            GUI_EVENT_KEY_ENTER      => $play_action,
            GUI_EVENT_KEY_PLAY       => $play_action,
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

        $movie_id = MediaURL::decode($user_input->selected_media_url)->movie_id;

        switch ($user_input->control_id) {
            case ACTION_ITEM_UP:
                $user_input->sel_ndx--;
                if ($user_input->sel_ndx < 0) {
                    $user_input->sel_ndx = 0;
                }
                return $this->plugin->vod->change_vod_favorites(PLUGIN_FAVORITES_OP_MOVE_UP, $movie_id);

            case ACTION_ITEM_DOWN:
                $num_favorites = $this->plugin->vod->get_favorite_movie_ids()->size();
                $user_input->sel_ndx++;
                if ($user_input->sel_ndx >= $num_favorites) {
                    $user_input->sel_ndx = $num_favorites - 1;
                }
                return $this->plugin->vod->change_vod_favorites(PLUGIN_FAVORITES_OP_MOVE_DOWN, $movie_id);

            case ACTION_ITEM_DELETE:
                $action = $this->plugin->vod->change_vod_favorites(PLUGIN_FAVORITES_OP_REMOVE, $movie_id);
                return ($this->plugin->vod->get_favorite_movie_ids()->size() !== 0)
                    ? $action
                    : Action_Factory::invalidate_all_folders(Action_Factory::close_and_run());

            case ACTION_ITEMS_CLEAR:
                $this->plugin->vod->change_vod_favorites(ACTION_ITEMS_CLEAR, null);
                return Action_Factory::invalidate_all_folders(Action_Factory::close_and_run());

            case GUI_EVENT_KEY_POPUP_MENU:
                $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_CLEAR, TR::t('clear_favorites'), "brush.png");
                return Action_Factory::show_popup_menu($menu_items);

            case GUI_EVENT_KEY_RETURN:
                return Action_Factory::invalidate_all_folders(Action_Factory::close_and_run());
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $movie_ids = $this->plugin->vod->get_favorite_movie_ids();

        $items = array();
        foreach ($movie_ids as $movie_id) {
            $this->plugin->vod->ensure_movie_loaded($movie_id);
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);

            if (is_null($short_movie)) {
                $caption = TR::t('vod_screen_no_film_info');
                $poster_url = "missing://";
            } else {
                $caption = $short_movie->name;
                $poster_url = $short_movie->poster_url;
            }

            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Vod_Movie_Screen::get_media_url_string($movie_id),
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array(
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

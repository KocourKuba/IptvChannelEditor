<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_History_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_history';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER    => $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play(),
            GUI_EVENT_KEY_PLAY     => Action_Factory::vod_play(),
            GUI_EVENT_KEY_B_GREEN  => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete')),
            GUI_EVENT_KEY_C_YELLOW => User_Input_Handler_Registry::create_action($this, ACTION_ITEMS_CLEAR, TR::t('clear_history')),
            GUI_EVENT_KEY_D_BLUE   => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
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
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);

        switch ($user_input->control_id)
		{
			case ACTION_ITEM_DELETE:
                //hd_debug_print("Delete movie_id: $media_url->movie_id");
                $this->plugin->vod->remove_movie_from_history($movie_id);
				$sel_ndx = $user_input->sel_ndx + 1;
				if ($sel_ndx < 0)
					$sel_ndx = 0;
				$range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
				return Action_Factory::update_regular_folder($range, true, $sel_ndx);

            case ACTION_ITEMS_CLEAR:
                $this->plugin->vod->set_history_movies(array());
                $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true);

            case ACTION_ADD_FAV:
				$is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
				$opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
				$message = $is_favorite ? TR::t('deleted_from_favorite') : TR::t('added_to_favorite');
				return Action_Factory::show_title_dialog($message,
                    $this->plugin->vod->change_vod_favorites($opt_type, $movie_id));
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
        hd_debug_print(null, true);

        $items = array();
        foreach ($this->plugin->vod->get_history_movies() as $movie_id => $movie_infos) {
            if (empty($movie_infos)) continue;

            hd_debug_print("history id: $movie_id, data" . json_encode($movie_infos));
            $this->plugin->vod->ensure_movie_loaded($movie_id);
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);

            if (is_null($short_movie)) {
                $detailed_info = $caption = TR::t('vod_screen_no_film_info');
                $poster_url = "missing://";
            } else {
                $caption = $short_movie->name;
                $last_viewed = 0;
                foreach ($movie_infos as $movie_info) {
                    if (isset($movie_info[Movie::WATCHED_DATE]) && $movie_info[Movie::WATCHED_DATE] > $last_viewed) {
                        $last_viewed = $movie_info[Movie::WATCHED_DATE];
                        $info = $movie_info;
                    }
                }

                if (isset($info) && $info[Movie::WATCHED_DATE] !== 0) {
                    hd_debug_print("detailed info: " . json_encode($info));
                    if ($info[Movie::WATCHED_FLAG]) {
                        $detailed_info = TR::t('vod_screen_all_viewed__2', $short_movie->name, format_datetime("d.m.Y H:i", $info[Movie::WATCHED_DATE]));
                    } else if ($info[Movie::WATCHED_DURATION] !== -1 && count($movie_infos) < 2) {
                        $percent = (int)((float)$info[Movie::WATCHED_POSITION] / (float)$info[Movie::WATCHED_DURATION] * 100);
                        $detailed_info = TR::t('vod_screen_last_viewed__3', $short_movie->name, format_datetime("d.m.Y H:i", $info[Movie::WATCHED_DATE]), $percent);
                    } else {
                        $detailed_info = TR::t('vod_screen_last_viewed__2', $short_movie->name, format_datetime("d.m.Y H:i", $info[Movie::WATCHED_DATE]));
                    }
                } else {
                    $detailed_info = $short_movie->name;
                }

                $poster_url = $short_movie->poster_url;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Movie_Screen::get_media_url_string($movie_id),
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $poster_url,
                    ViewItemParams::item_detailed_info => $detailed_info,
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
            $this->plugin->get_screen_view('list_1x11_small_info'),
            $this->plugin->get_screen_view('list_1x11_normal_info'),
        );
    }
}

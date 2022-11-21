<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_Vod_History_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_history';

    const ACTION_ADD_FAV = 'add_favorite';
    const ACTION_POPUP_MENU = 'popup_menu';
    const ACTION_ITEM_DELETE = 'item_delete';

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

        if ($plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();

        $actions[GUI_EVENT_KEY_ENTER] = $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play();
        $actions[GUI_EVENT_KEY_PLAY] = Action_Factory::vod_play();

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_DELETE);
        $add_action['caption'] = 'Удалить';
        $actions[GUI_EVENT_KEY_B_GREEN] = $add_action;

        $add_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ADD_FAV);
        $add_favorite_action['caption'] = 'В Избранное';
        $actions[GUI_EVENT_KEY_D_BLUE] = $add_favorite_action;

        $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, self::ACTION_POPUP_MENU);

        return $actions;
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
        //hd_print("Vod_History_Screen: handle_user_input:");
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id)
		{
			case self::ACTION_ITEM_DELETE:
				$media_url = MediaURL::decode($user_input->selected_media_url);
                //hd_print("Vod_History_Screen: Delete movie_id: $media_url->movie_id");
                $this->plugin->vod->remove_movie_from_history($media_url->movie_id, $plugin_cookies);
				$parent_media_url = MediaURL::decode($user_input->parent_media_url);
				$sel_ndx = $user_input->sel_ndx + 1;
				if ($sel_ndx < 0)
					$sel_ndx = 0;
				$range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
				return Action_Factory::update_regular_folder($range, true, $sel_ndx);

			case self::ACTION_POPUP_MENU:
				$add_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ADD_FAV);
				$caption = $this->plugin->vod->is_favorite_movie_id($movie_id) ? 'Удалить из Избранного' : 'Добавить в избранное';
				$menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_favorite_action);
				return Action_Factory::show_popup_menu($menu_items);

			case self::ACTION_ADD_FAV:
				$is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
				$opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
				$message = $is_favorite ? 'Удалено из Избранного' : 'Добавлено в Избранное';
				$this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);
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
        //hd_print("Starnet_Vod_History_Screen: get_all_folder_items");

        $this->plugin->vod->ensure_history_loaded($plugin_cookies);
        $history_items = $this->plugin->vod->get_history_movies();

        $items = array();
        foreach ($history_items as $movie_id => $movie_infos) {
            if (empty($movie_infos)) continue;

            $this->plugin->vod->ensure_movie_loaded($movie_id, $plugin_cookies);
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);

            if (is_null($short_movie)) {
                $caption = "Информация о фильме недоступна";
                $poster_url = "missing://";
            } else {
                $last_viewed = 0;
                foreach ($movie_infos as $info) {
                    if (isset($info[Movie::WATCHED_DATE])) {
                        $last_viewed = max($last_viewed, $info[Movie::WATCHED_DATE]);
                        //hd_print("get_all_folder_items: info $key => {$info[Movie::WATCHED_DATE]}");
                    }
                }

                $caption = $short_movie->name;
                if ($last_viewed !== 0)
                    $caption .= "|Последний просмотр: " . format_datetime("d.m.Y H:i", $last_viewed) . "|";
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
}

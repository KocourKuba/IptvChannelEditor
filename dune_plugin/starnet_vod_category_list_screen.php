<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod_category.php';
require_once 'starnet_vod_list_screen.php';

class Starnet_Vod_Category_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_category_list';

    const FAV_MOVIES_GROUP_CAPTION = 'plugin_favorites';
    const FAV_MOVIES_GROUP_ICON = 'plugin_file://icons/fav_movie.png';

    const SEARCH_MOVIES_GROUP_CAPTION = 'search';
    const SEARCH_MOVIES_GROUP_ICON = 'plugin_file://icons/search_movie.png';

    const FILTER_MOVIES_GROUP_CAPTION = 'filter';
    const FILTER_MOVIES_GROUP_ICON = 'plugin_file://icons/filter_movie.png';

    const HISTORY_MOVIES_GROUP_CAPTION = 'plugin_history';
    const HISTORY_MOVIES_GROUP_ICON = 'plugin_file://icons/history_movie.png';

    /**
     * @var array
     */
    private $category_list;

    /**
     * @var array
     */
    private $category_index;

    /**
     * @param string $category_id
     * @return false|string
     */
    public static function get_media_url_string($category_id)
    {
        return MediaURL::encode(array('screen_id' => static::ID, 'category_id' => $category_id,));
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array(
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_C_YELLOW => User_Input_Handler_Registry::create_action($this, ACTION_RELOAD, TR::t('vod_screen_reload_playlist')),
        );

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_ENGINE) === "M3U") {
            $all_vod_lists = $this->plugin->config->get_vod_list_names($current_idx);
            if (count($all_vod_lists) > 1) {
                $change_playlist = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU);
                $change_playlist['caption'] = TR::t('vod_screen_change_playlist');
                if (!is_not_certified()) {
                    $actions[GUI_EVENT_KEY_POPUP_MENU] = $change_playlist;
                } else {
                    $actions[GUI_EVENT_KEY_B_GREEN] = $change_playlist;
                }
            }
        }

        return $actions;
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
            case ACTION_RELOAD:
                hd_debug_print("reload categories");
                $this->clear_vod();
                $media_url = MediaURL::decode($user_input->parent_media_url);
                $range = $this->get_folder_range($media_url, 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true, -1);

            case ACTION_CHANGE_PLAYLIST:
                $current_idx = $this->plugin->get_parameter(PARAM_VOD_IDX, 0);

                if (isset($user_input->{PARAM_PLAYLIST})
                    && $user_input->{PARAM_PLAYLIST} !== false
                    && $user_input->{PARAM_PLAYLIST} !== $current_idx) {
                    $this->plugin->set_parameter(PARAM_VOD_IDX, $user_input->{PARAM_PLAYLIST});
                    hd_debug_print("change VOD playlist to: " . $user_input->{PARAM_PLAYLIST});
                    return User_Input_Handler_Registry::create_action($this, ACTION_RELOAD);
                }
                break;

            case GUI_EVENT_KEY_POPUP_MENU;
                $menu_items = array();
                $all_vod_lists = $this->plugin->config->get_vod_list_names($current_idx);
                foreach ($all_vod_lists as $idx => $list) {
                    $add_param[PARAM_PLAYLIST] = $idx;

                    $icon_url = null;
                    if ($idx === (int)$current_idx) {
                        $icon_url = "gui_skin://small_icons/playlist_file.aai";
                    }
                    $menu_items[] = User_Input_Handler_Registry::create_popup_item($this, ACTION_CHANGE_PLAYLIST, $list, $icon_url, $add_param);
                }

                return Action_Factory::show_popup_menu($menu_items);
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

        if (is_null($this->category_index) || is_null($this->category_list)) {
            $this->plugin->config->fetchVodCategories($this->category_list, $this->category_index);
        }

        $category_list = $this->category_list;

        if (isset($media_url->category_id)) {
            if (!isset($this->category_index[$media_url->category_id])) {
                hd_debug_print("Error: parent category (id: $media_url->category_id) not found.");
                throw new Exception('No parent category found');
            }

            $parent_category = $this->category_index[$media_url->category_id];
            $category_list = $parent_category->get_sub_categories();
        }

        $items = array();

        // Favorites
        if (!isset($media_url->category_id)) {
            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Vod_Favorites_Screen::get_media_url_str(),
                PluginRegularFolderItem::caption => TR::t(self::FAV_MOVIES_GROUP_CAPTION),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => self::FAV_MOVIES_GROUP_ICON,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GOLD, // Light yellow
                    ViewItemParams::item_detailed_icon_path => self::FAV_MOVIES_GROUP_ICON,
                )
            );

            // History
            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Vod_History_Screen::get_media_url_str(),
                PluginRegularFolderItem::caption => TR::t(self::HISTORY_MOVIES_GROUP_CAPTION),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => self::HISTORY_MOVIES_GROUP_ICON,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_TURQUOISE, // Cyan
                    ViewItemParams::item_detailed_icon_path => self::HISTORY_MOVIES_GROUP_ICON,
                )
            );

            // Search
            $items[] = array(
                PluginRegularFolderItem::media_url => Starnet_Vod_Search_Screen::get_media_url_str(),
                PluginRegularFolderItem::caption => TR::t(self::SEARCH_MOVIES_GROUP_CAPTION),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => self::SEARCH_MOVIES_GROUP_ICON,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GREEN, // Green
                    ViewItemParams::item_detailed_icon_path => self::SEARCH_MOVIES_GROUP_ICON,
                )
            );

            // Filter
            if ($this->plugin->config->get_feature(Plugin_Constants::VOD_FILTER_SUPPORTED)) {
                $items[] = array(
                    PluginRegularFolderItem::media_url => Starnet_Vod_Filter_Screen::get_media_url_str(),
                    PluginRegularFolderItem::caption => TR::t(self::FILTER_MOVIES_GROUP_CAPTION),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => self::FILTER_MOVIES_GROUP_ICON,
                        ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GREEN, // Green
                        ViewItemParams::item_detailed_icon_path => self::FILTER_MOVIES_GROUP_ICON,
                    )
                );
            }
        }

        if (!empty($category_list)) {
            foreach ($category_list as $category) {
                $category_id = $category->get_id();
                if (!is_null($category->get_sub_categories())) {
                    $media_url_str = self::get_media_url_string($category_id);
                } else if ($category_id === Vod_Category::FLAG_ALL
                    || $category_id === Vod_Category::FLAG_SEARCH
                    || $category_id === Vod_Category::FLAG_FILTER) {
                    // special category id's
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_string($category_id, null);
                } else if ($category->get_parent() !== null) {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_string($category->get_parent()->get_id(), $category_id);
                } else {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_string($category_id, null);
                }

                $items[] = array(
                    PluginRegularFolderItem::media_url => $media_url_str,
                    PluginRegularFolderItem::caption => $category->get_caption(),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => $category->get_icon_path(),
                        ViewItemParams::item_detailed_icon_path => $category->get_icon_path(),
                    )
                );
            }
        }

        return $items;
    }

    /**
     * Clear vod information
     * @return void
     */
    public function clear_vod()
    {
        unset($this->category_list, $this->category_index);
        $this->plugin->vod->clear_movie_cache();
        $this->plugin->config->ClearPlaylistCache(false);
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('list_1x12_vod_info_normal'),
        );
    }
}

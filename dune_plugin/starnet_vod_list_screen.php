<?php

require_once 'lib/abstract_regular_screen.php';
require_once 'lib/vod/short_movie_range.php';
require_once 'starnet_vod_search_screen.php';

class Starnet_Vod_List_Screen extends Abstract_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_list';

    const ACTION_SETTINGS = 'settings';
    const ACTION_ADD_FAV = 'add_favorite';
    const ACTION_POPUP_MENU = 'popup_menu';
    const ACTION_CREATE_SEARCH = 'create_search';
    const ACTION_NEW_SEARCH = 'new_search';
    const ACTION_RUN_SEARCH = 'run_search';

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
     * @param string $category_id
     * @param string $genre_id
     * @param string $name
     * @return false|string
     */
    public static function get_media_url_str($category_id, $genre_id, $name = false)
    {
        $arr['screen_id'] = self::ID;
        $arr['category_id'] = $category_id;
        $arr['genre_id'] = $genre_id;
        if ($name !== false) {
            $arr['name'] = $name;
        }

        return MediaURL::encode($arr);
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();

        $add_settings = User_Input_Handler_Registry::create_action($this, self::ACTION_SETTINGS);
        $add_settings['caption'] = 'Настройки плагина';
        $actions[GUI_EVENT_KEY_SETUP] = $add_settings;
        $actions[GUI_EVENT_KEY_B_GREEN] = $add_settings;

        $actions[GUI_EVENT_KEY_ENTER] = $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play();

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_CREATE_SEARCH);
        $add_action['caption'] = 'Поиск';
        $actions[GUI_EVENT_KEY_SEARCH] = $add_action;
        $actions[GUI_EVENT_KEY_C_YELLOW] = $add_action;

        $add_favorite_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ADD_FAV);
        $add_favorite_action['caption'] = 'В Избранное';
        $actions[GUI_EVENT_KEY_D_BLUE] = $add_favorite_action;

        $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, self::ACTION_POPUP_MENU);

        return $actions;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('VodListScreen: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id) {
            case self::ACTION_CREATE_SEARCH:
                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null, self::ACTION_NEW_SEARCH, '',
                    $media_url->name, false, false, true, true, 1300, false, true);
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog('Поиск', $defs, true);

            case self::ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(User_Input_Handler_Registry::create_action($this, self::ACTION_RUN_SEARCH));

            case self::ACTION_RUN_SEARCH:
                $search_string = $user_input->{self::ACTION_NEW_SEARCH};
                HD::put_item(Starnet_Vod_Search_Screen::SEARCH_ITEM, $search_string);
                $search_items = HD::get_items(Starnet_Vod_Search_Screen::SEARCH_LIST);
                $k = array_search($search_string, $search_items);
                if ($k !== false) {
                    unset ($search_items [$k]);
                }

                array_unshift($search_items, $search_string);
                HD::put_items(Starnet_Vod_Search_Screen::SEARCH_LIST, $search_items);
                return Action_Factory::invalidate_folders(
                    array(Starnet_Vod_Search_Screen::ID),
                    Action_Factory::open_folder(
                        static::get_media_url_str(Vod_Category::FLAG_SEARCH, $search_string),
                        "Поиск: " . $search_string));

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
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return Short_Movie_Range
     */
    protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        hd_print("get_short_movie_range: '$media_url->category_id', from_idx: $from_ndx");
        $this->plugin->config->try_reset_pages();
        if (empty($media_url->genre_id)) {
            $key = $media_url->category_id;
        } else {
            $key = $media_url->category_id . "_" . $media_url->genre_id;
        }

        if ($media_url->category_id === Vod_Category::FLAG_SEARCH) {
            $movies = $this->plugin->config->getSearchList($media_url->genre_id, $plugin_cookies);
        } else if ($media_url->category_id === Vod_Category::FLAG_FILTER) {
            $movies = $this->plugin->config->getFilterList($media_url->genre_id, $plugin_cookies);
        } else if ($media_url->category_id === Vod_Category::FLAG_ALL || empty($media_url->genre_id)) {
            $movies = $this->plugin->config->getMovieList($media_url->category_id, $plugin_cookies);
        } else {
            $movies = $this->plugin->config->getMovieList($key, $plugin_cookies);
        }

        $count = count($movies);
        if (!$count) {
            return new Short_Movie_Range(0, 0);
        }

        $this->plugin->config->add_movie_counter($key, $count);
        return new Short_Movie_Range($from_ndx, $this->plugin->config->get_movie_counter($key), $movies);
    }

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
     */
    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        //hd_print("get_folder_range: $from_ndx");
        $movie_range = $this->get_short_movie_range($media_url, $from_ndx, $plugin_cookies);

        $total = $movie_range->total;
        if ($total <= 0) {
            return HD::create_regular_folder_range(array());
        }

        $items = array();
        foreach ($movie_range->short_movies as $movie) {
            $media_url_str = Starnet_Vod_Movie_Screen::get_media_url_str($movie->id, $movie->name, $movie->poster_url, $movie->info);
            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $movie->name,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $movie->poster_url,
                    ViewItemParams::item_detailed_info => $movie->info,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_WHITE,
                ),
                PluginRegularFolderItem::starred => $this->plugin->vod->is_favorite_movie_id($movie->id),
            );

            $this->plugin->vod->set_cached_short_movie(new Short_Movie($movie->id, $movie->name, $movie->poster_url));
        }

        return HD::create_regular_folder_range($items, $movie_range->from_ndx, $total, true);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("get_folder_view");
        $this->plugin->config->reset_movie_counter();
        $this->plugin->vod->clear_movie_cache();
        $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);

        return parent::get_folder_view($media_url, $plugin_cookies);
    }
}

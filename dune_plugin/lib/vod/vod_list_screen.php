<?php
require_once 'lib/abstract_regular_screen.php';

abstract class Vod_List_Screen extends Abstract_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_list';

    /**
     * @param Default_Dune_Plugin $plugin
     */
    protected function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_list_folder_views());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @param string $category_id
     * @param string $genre_id
     * @return false|string
     */
    public static function get_media_url_str($category_id, $genre_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'category_id' => $category_id,
                'genre_id' => $genre_id,
            ));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();

        $actions[GUI_EVENT_KEY_ENTER] = $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play();

        $add_action = User_Input_Handler_Registry::create_action($this, 'search');
        $add_action['caption'] = 'Поиск';
        $actions[GUI_EVENT_KEY_C_YELLOW] = $add_action;

        if ($this->plugin->vod->is_favorites_supported()) {
            $add_favorite_action = User_Input_Handler_Registry::create_action($this, 'add_favorite');
            $add_favorite_action['caption'] = 'В Избранное';
            $actions[GUI_EVENT_KEY_D_BLUE] = $add_favorite_action;

            $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, 'popup_menu');
        }

        return $actions;
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
        // hd_print('VodListScreen: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id) {
            case 'search':
                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null, 'new_search', '',
                    $media_url->name, 0, 0, 1, 1, 1300, 0, true);
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog('Поиск', $defs, true);

            case 'new_search':
                return Action_Factory::close_dialog_and_run(
                    User_Input_Handler_Registry::create_action($this, 'run_search'));

            case 'run_search':
                HD::put_item('search_item', $user_input->do_new_search);
                $search_items = HD::get_items('search_items');
                $k = array_search($user_input->do_new_search, $search_items);
                if ($k !== false) {
                    unset ($search_items [$k]);
                }

                array_unshift($search_items, $user_input->do_new_search);
                HD::put_items('search_items', $search_items);
                return Action_Factory::invalidate_folders(
                    array('search_screen'),
                    Action_Factory::open_folder(
                        static::get_media_url_str('search', $user_input->do_new_search),
                        "Поиск: " . $user_input->do_new_search));

            case 'popup_menu':
                $add_favorite_action = User_Input_Handler_Registry::create_action($this, 'add_favorite');
                $caption = $this->plugin->vod->is_favorite_movie_id($movie_id) ? 'Удалить из Избранного' : 'Добавить в избранное';
                $menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_favorite_action);
                return Action_Factory::show_popup_menu($menu_items);

            case 'add_favorite':
                $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
                $opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $message = $is_favorite ? 'Удалено из Избранного' : 'Добавлено в Избранное';
                $this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);
                $parent_url = MediaURL::decode($user_input->parent_media_url);
                return Action_Factory::invalidate_folders(
                    array(self::get_media_url_str($parent_url->category_id, $parent_url->genre_id)),
                    Action_Factory::show_title_dialog($message));
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return Short_Movie_Range
     */
    abstract protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies);

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
     */
    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        // hd_print("get_folder_range: $from_ndx");
        $movie_range = $this->get_short_movie_range($media_url, $from_ndx, $plugin_cookies);

        $total = $movie_range->total;
        if ($total <= 0) {
            return HD::create_regular_folder_range(array());
        }

        $items = array();
        foreach ($movie_range->short_movies as $movie) {
            $media_url_str = Vod_Movie_Screen::get_media_url_str($movie->id, $movie->name, $movie->poster_url, $movie->info);
            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $movie->name,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $movie->poster_url,
                    ViewItemParams::item_detailed_info => $movie->info,
                    ViewItemParams::item_caption_color => 15,
                ),
                PluginRegularFolderItem::starred => $this->plugin->vod->is_favorite_movie_id($movie->id),
            );

            $this->plugin->vod->set_cached_short_movie(new Short_Movie($movie->id, $movie->name, $movie->poster_url));
        }

        return HD::create_regular_folder_range($items, $movie_range->from_ndx, $total, true);
    }

    /**
     * @param MediaURL $media_url
     * @return null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
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

<?php
require_once 'lib/abstract_regular_screen.php';

abstract class VodListScreen extends AbstractRegularScreen implements UserInputHandler
{
    const ID = 'vod_list';

    protected function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_list_folder_views());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

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

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();

        $actions[GUI_EVENT_KEY_ENTER] = $this->plugin->vod->is_movie_page_supported() ? ActionFactory::open_folder() : ActionFactory::vod_play();

        $add_action = UserInputHandlerRegistry::create_action($this, 'search');
        $add_action['caption'] = 'Поиск';
        $actions[GUI_EVENT_KEY_C_YELLOW] = $add_action;

        if ($this->plugin->vod->is_favorites_supported()) {
            $add_favorite_action = UserInputHandlerRegistry::create_action($this, 'add_favorite');
            $add_favorite_action['caption'] = 'В Избранное';
            $actions[GUI_EVENT_KEY_D_BLUE] = $add_favorite_action;

            $actions[GUI_EVENT_KEY_POPUP_MENU] = UserInputHandlerRegistry::create_action($this, 'popup_menu');
        }

        return $actions;
    }

    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

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
                ControlFactory::add_text_field($defs,
                    $this, null, 'new_search', '',
                    $media_url->name, 0, 0, 1, 1, 1300, 0, true);
                ControlFactory::add_vgap($defs, 500);
                return ActionFactory::show_dialog('Поиск', $defs, true);

            case 'new_search':
                return ActionFactory::close_dialog_and_run(
                    UserInputHandlerRegistry::create_action($this, 'run_search'));

            case 'run_search':
                HD::put_item('search_item', $user_input->do_new_search);
                $search_items = HD::get_items('search_items');
                $k = array_search($user_input->do_new_search, $search_items);
                if ($k !== false) {
                    unset ($search_items [$k]);
                }

                array_unshift($search_items, $user_input->do_new_search);
                HD::put_items('search_items', $search_items);
                return ActionFactory::invalidate_folders(
                    array('search_screen'),
                    ActionFactory::open_folder(
                        static::get_media_url_str('search', $user_input->do_new_search),
                        "Поиск: " . $user_input->do_new_search));

            case 'popup_menu':
                $add_favorite_action = UserInputHandlerRegistry::create_action($this, 'add_favorite');
                $caption = $this->plugin->vod->is_favorite_movie_id($movie_id) ? 'Удалить из Избранного' : 'Добавить в избранное';
                $menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_favorite_action);
                return ActionFactory::show_popup_menu($menu_items);

            case 'add_favorite':
                $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
                $opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $message = $is_favorite ? 'Удалено из Избранного' : 'Добавлено в Избранное';
                $this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);

                return ActionFactory::show_title_dialog($message);
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    // Returns ShortMovieRange.
    abstract protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies);

    /**
     * @throws Exception
     */
    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        // hd_print("get_folder_range: $from_ndx");
        $movie_range = $this->get_short_movie_range($media_url, $from_ndx, $plugin_cookies);

        $total = (int)$movie_range->total;
        if ($total <= 0) {
            return HD::create_regular_folder_range(array());
        }

        $items = array();
        foreach ($movie_range->short_movies as $movie) {
            $media_url_str = VodMovieScreen::get_media_url_str($movie->id, $movie->name, $movie->poster_url, $movie->info);
            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $movie->name,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $movie->poster_url,
                    ViewItemParams::item_detailed_info => $movie->info,
                    ViewItemParams::item_caption_color => 15,
                )
            );

            $this->plugin->vod->set_cached_short_movie(new ShortMovie($movie->id, $movie->name, $movie->poster_url));
        }

        return HD::create_regular_folder_range($items, $movie_range->from_ndx, $total, true);
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }

    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("get_folder_view");
        $this->plugin->config->reset_movie_counter();
        $this->plugin->vod->clear_movie_cache();
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);
        $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);

        return parent::get_folder_view($media_url, $plugin_cookies);
    }
}

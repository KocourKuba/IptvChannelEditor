<?php

require_once 'lib/abstract_regular_screen.php';
require_once 'lib/short_movie_range.php';
require_once 'lib/user_input_handler_registry.php';
require_once 'starnet_vod_search_screen.php';

class Starnet_Vod_List_Screen extends Abstract_Regular_Screen
{
    const ID = 'vod_list';

    /**
     * @param string $category_id
     * @param string $genre_id
     * @return false|string
     */
    public static function get_media_url_string($category_id, $genre_id)
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'category_id' => $category_id, 'genre_id' => $genre_id));
    }

    /**
     * Opens the search result, or the movie itself if only one movie is found,
     * or shows a dialog if nothing is found
     *
     * @param Default_Dune_Plugin $plugin
     * @param string $search_string plain string or json encoded array with title and original_title
     * @return array
     */
    public static function make_search_action($plugin, $search_string)
    {
        $movies = $plugin->config->prefetchSearchList($search_string);
        if (empty($movies)) {
            $plugin->config->takePrefetchedSearchList($search_string);
            hd_debug_print("Nothing found: $search_string");
            return Action_Factory::show_title_dialog(TR::t('vod_search_not_found'));
        }

        if (count($movies) === 1 && $plugin->vod->is_movie_page_supported()) {
            $plugin->config->takePrefetchedSearchList($search_string);
            $movie = $movies[0];
            hd_debug_print("Only one movie found, open it: $movie->id");
            $plugin->vod->set_cached_short_movie($movie);
            return Action_Factory::open_folder(
                Starnet_Vod_Movie_Screen::get_media_url_string($movie->id, $movie->name, $movie->poster_url, $movie->info),
                $movie->name
            );
        }

        return Action_Factory::open_folder(static::get_media_url_string(Vod_Category::FLAG_SEARCH, $search_string),
            TR::t('search__1', implode(' / ', default_config::decode_search_keywords($search_string))));
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $add_action = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, TR::t('search'));

        return array(
            GUI_EVENT_KEY_ENTER => $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play(),
            GUI_EVENT_KEY_SEARCH => $add_action,
            GUI_EVENT_KEY_C_YELLOW => $add_action,
            GUI_EVENT_KEY_D_BLUE => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
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

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id) {
            case ACTION_CREATE_SEARCH:
                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, ACTION_NEW_SEARCH, '', $media_url->name,
                    false, false, true, true,
                    Control_Factory::DLG_MAX_CONTROLS_WIDTH, false, true);
                Control_Factory::add_vgap($defs, 500);
                return Action_Factory::show_dialog(TR::t('search'), $defs, true);

            case ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(User_Input_Handler_Registry::create_action($this, ACTION_RUN_SEARCH));

            case ACTION_RUN_SEARCH:
                $search_string = $user_input->{ACTION_NEW_SEARCH};
                $search_items = HD::get_data_items(Starnet_Vod_Search_Screen::VOD_SEARCH_LIST);
                $k = array_search($search_string, $search_items);
                if ($k !== false) {
                    unset ($search_items [$k]);
                }

                array_unshift($search_items, $search_string);
                HD::put_data_items(Starnet_Vod_Search_Screen::VOD_SEARCH_LIST, $search_items);
                return Action_Factory::invalidate_folders(array(Starnet_Vod_Search_Screen::get_media_url_str()),
                    static::make_search_action($this->plugin, $search_string));

            case ACTION_ADD_FAV:
                $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
                $opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $this->plugin->vod->change_vod_favorites($opt_type, $movie_id);
                $this->plugin->vod->save_movie_favorites();
                $urls[] = $user_input->parent_media_url;
                $urls[] = Starnet_Vod_Favorites_Screen::get_media_url_str();

                return Action_Factory::invalidate_folders($urls);
        }

        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
     */
    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print("from_ndx: $from_ndx, MediaURL: $media_url", true);

        $this->plugin->config->try_reset_pages();
        if (empty($media_url->genre_id) || $media_url->category_id === Vod_Category::FLAG_ALL_MOVIES) {
            $key = $media_url->category_id;
        } else {
            $key = $media_url->category_id . "_" . $media_url->genre_id;
        }

        if ($media_url->category_id === Vod_Category::FLAG_SEARCH) {
            // the first page was already fetched by make_search_action()
            $movies = $from_ndx === 0 ? $this->plugin->config->takePrefetchedSearchList($media_url->genre_id) : null;
            if (is_null($movies)) {
                $movies = $this->plugin->config->getSearchResults($media_url->genre_id);
            }
        } else if ($media_url->category_id === Vod_Category::FLAG_FILTER) {
            $movies = $this->plugin->config->getFilterList($media_url->genre_id);
        } else {
            $movies = $this->plugin->config->getMovieList($key);
        }

        $count = count($movies);
        if ($count) {
            $this->plugin->config->add_movie_counter($key, $count);
            $movie_range = new Short_Movie_Range($from_ndx, $this->plugin->config->get_movie_counter($key), $movies);
        } else {
            $movie_range = new Short_Movie_Range(0, 0);
        }

        $total = $movie_range->total;
        if ($total <= 0) {
            return $this->create_regular_folder_range(array());
        }

        $items = array();
        foreach ($movie_range->short_movies as $movie) {
            $media_url_str = Starnet_Vod_Movie_Screen::get_media_url_string($movie->id, $movie->name, $movie->poster_url, $movie->info);
            $items[] = array(
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $movie->name,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $movie->poster_url,
                    ViewItemParams::item_detailed_info => $movie->info,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_WHITE,
                ),
                PluginRegularFolderItem::starred => $this->plugin->vod->is_favorite_movie_id($movie->id),
            );

            $this->plugin->vod->set_cached_short_movie(new Short_Movie($movie->id, $movie->name, $movie->poster_url, $movie->info));
        }

        return $this->create_regular_folder_range($items, $movie_range->from_ndx, $total, true);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        // the first page of a search was already fetched by make_search_action()
        $this->plugin->config->reset_movie_counter($media_url->category_id === Vod_Category::FLAG_SEARCH
            && $this->plugin->config->hasPrefetchedSearchList($media_url->genre_id));
        $this->plugin->vod->clear_movie_cache();

        return parent::get_folder_view($media_url, $plugin_cookies);
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('icons_5x2_movie_no_caption'),
            $this->plugin->get_screen_view('list_1x10_movie_info_normal'),
        );
    }
}

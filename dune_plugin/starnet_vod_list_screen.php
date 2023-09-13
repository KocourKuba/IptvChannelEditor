<?php

require_once 'lib/abstract_regular_screen.php';
require_once 'lib/short_movie_range.php';
require_once 'starnet_vod_search_screen.php';

class Starnet_Vod_List_Screen extends Abstract_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_list';

    /**
     * @param string $category_id
     * @param string $genre_id
     * @param string $name
     * @return false|string
     */
    public static function get_media_url_string($category_id, $genre_id, $name = false)
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
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $add_action = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, TR::t('search'));

        return array(
            GUI_EVENT_KEY_ENTER      => $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play(),
            GUI_EVENT_KEY_SEARCH     => $add_action,
            GUI_EVENT_KEY_C_YELLOW   => $add_action,
            GUI_EVENT_KEY_D_BLUE     => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
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

        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id) {
            case ACTION_CREATE_SEARCH:
                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null, ACTION_NEW_SEARCH, '',
                    $media_url->name, false, false, true, true, 1300, false, true);
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
                return Action_Factory::invalidate_folders(
                    array(Starnet_Vod_Search_Screen::ID),
                    Action_Factory::open_folder(
                        static::get_media_url_string(Vod_Category::FLAG_SEARCH, $search_string),
                        TR::t('search') . ": $search_string"));

            case ACTION_ADD_FAV:
                $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
                $opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);

                return Action_Factory::invalidate_folders(array(self::get_media_url_string($parent_media_url->category_id, $parent_media_url->genre_id)));
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
        //hd_debug_print("$from_ndx");
        hd_debug_print("'$media_url->category_id', from_idx: $from_ndx");
        $this->plugin->config->try_reset_pages();
        if (empty($media_url->genre_id) || $media_url->category_id === Vod_Category::FLAG_ALL) {
            $key = $media_url->category_id;
        } else {
            $key = $media_url->category_id . "_" . $media_url->genre_id;
        }

        $movies = array();

        if ($media_url->category_id === Vod_Category::FLAG_SEARCH) {
            if ($from_ndx === 0) {
                $movies = $this->plugin->config->getSearchList($media_url->genre_id);
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

        return $this->create_regular_folder_range($items, $movie_range->from_ndx, $total, true);
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->config->reset_movie_counter();
        $this->plugin->vod->clear_movie_cache();
        $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);

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

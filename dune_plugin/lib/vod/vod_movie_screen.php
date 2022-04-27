<?php
require_once 'lib/abstract_controls_screen.php';
require_once 'lib/user_input_handler.php';
require_once 'default_config.php';
require_once 'vod.php';
require_once 'vod_series_list_screen.php';
require_once 'vod_variants_list_screen.php';

class Vod_Movie_Screen extends Abstract_Controls_Screen implements User_Input_Handler
{
    const ID = 'vod_movie';

    /**
     * @param string $movie_id
     * @param string|false $name
     * @param string|false $poster_url
     * @param string|false $info
     * @return false|string
     */
    public static function get_media_url_str($movie_id, $name = false, $poster_url = false, $info = false)
    {
        $arr['screen_id'] = self::ID;
        $arr['movie_id'] = $movie_id;
        if ($name !== false) {
            $arr['name'] = $name;
        }
        if ($poster_url !== false) {
            $arr['poster_url'] = $poster_url;
        }
        if ($info !== false) {
            $arr['info'] = $info;
        }

        //hd_print("Movie ID: $movie_id, Movie name: $name, Movie Poster: $poster_url");

        return MediaURL::encode($arr);
    }

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin);

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $plugin->create_screen($this);
        }
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
    public function get_control_defs(MediaURL $media_url, &$plugin_cookies)
    {
        return array();
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            hd_print("empty movie");
            return null;
        }

        if (empty($movie->series_list)) {
            hd_print("empty series");
            return null;
        }

        $has_right_button = $this->plugin->vod->is_favorites_supported();
        $right_button_caption = null;
        $right_button_action = null;
        if ($has_right_button) {
            $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);
            $right_button_caption = $this->plugin->vod->is_favorite_movie_id($movie->id) ? 'Удалить из Избранного' : 'Добавить в Избранное';
            $right_button_action = User_Input_Handler_Registry::create_action($this, 'favorites', array('movie_id' => $movie->id));
        }

        if (isset($movie->season_list)) {
            $multiple = !empty($movie->season_list);
            $screen_media_url = Vod_Seasons_List_Screen::get_media_url_str($movie->id);
        } else if (count($movie->series_list) > 1) {
            $multiple = true;
            $screen_media_url = Vod_Series_List_Screen::get_media_url_str($movie->id);
        } else if ($movie->has_variants()) {
            $multiple = true;
            $screen_media_url = Vod_Variants_List_Screen::get_media_url_str($movie->id);
        } else {
            $multiple = false;
            $screen_media_url = Vod_Series_List_Screen::get_media_url_str($movie->id);
        }

        //hd_print("Vod_Movie_Screen: multiple: " . ($multiple ? "true" : "false") . " MediaUrl $screen_media_url");
        $movie_folder_view = array
        (
            PluginMovieFolderView::movie => $movie->get_movie_array(),
            PluginMovieFolderView::has_right_button => $has_right_button,
            PluginMovieFolderView::right_button_caption => $right_button_caption,
            PluginMovieFolderView::right_button_action => $right_button_action,
            PluginMovieFolderView::has_multiple_series => $multiple,
            PluginMovieFolderView::series_media_url => $screen_media_url,
            PluginMovieFolderView::params => array
            (
                PluginFolderViewParams::paint_path_box => false,
                PluginFolderViewParams::paint_content_box_background => true,
                PluginFolderViewParams::background_url => $this->plugin->config->GET_BG_PICTURE()
            )
        );

        return array
        (
            PluginFolderView::multiple_views_supported => false,
            PluginFolderView::archive => null,
            PluginFolderView::view_kind => PLUGIN_FOLDER_VIEW_MOVIE,
            PluginFolderView::data => $movie_folder_view,
        );
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Movie: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if ($user_input->control_id === 'favorites') {
            $movie_id = $user_input->movie_id;

            $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
            $opt_type = $this->plugin->vod->is_favorite_movie_id($movie_id) ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
            $this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);

            $message = $is_favorite ? 'Удалено из Избранного' : 'Добавлено в Избранное';

            return Action_Factory::show_title_dialog($message,
                Action_Factory::invalidate_folders(array(
                        self::get_media_url_str($movie_id),
                        Vod_Favorites_Screen::get_media_url_str())
                )
            );
        }

        return null;
    }
}

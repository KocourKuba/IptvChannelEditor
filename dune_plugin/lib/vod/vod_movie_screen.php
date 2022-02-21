<?php
require_once 'lib/screen.php';
require_once 'default_config.php';
require_once 'vod.php';
require_once 'vod_series_list_screen.php';

class VodMovieScreen implements Screen, UserInputHandler
{
    const ID = 'vod_movie';
    protected $plugin;

    public function __construct(DefaultDunePlugin $plugin)
    {
        $this->plugin = $plugin;

        if ($this->plugin->config->get_vod_support()) {
            $this->plugin->create_screen($this);
            UserInputHandlerRegistry::get_instance()->register_handler($this);
        }
    }

    public static function get_media_url_str($movie_id, $name = false, $poster_url = false, $info = false) {
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

    public function get_id()
    {
        return self::ID;
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);

        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            hd_print("empty movie");
            return null;
        }

        $has_right_button = $this->plugin->vod->is_favorites_supported();
        $right_button_caption = null;
        $right_button_action = null;
        if ($has_right_button) {
            $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);
            $right_button_caption = $this->plugin->vod->is_favorite_movie_id($movie->id) ? 'Удалить из Избранного' : 'Добавить в Избранное';
            $right_button_action = UserInputHandlerRegistry::create_action($this, 'favorites', array('movie_id' => $movie->id));
        }

        $movie_folder_view = array
        (
            PluginMovieFolderView::movie => $movie->get_movie_array(),
            PluginMovieFolderView::has_right_button => $has_right_button,
            PluginMovieFolderView::right_button_caption => $right_button_caption,
            PluginMovieFolderView::right_button_action => $right_button_action,
            PluginMovieFolderView::has_multiple_series => (count($movie->series_list) > 1),
            PluginMovieFolderView::series_media_url =>
                VodSeriesListScreen::get_media_url_str($movie->id),
                PluginMovieFolderView::params => array
                (			               
                    PluginFolderViewParams::paint_path_box =>false,
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

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Movie: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if ($user_input->control_id === 'favorites') {
            $movie_id = $user_input->movie_id;

            $is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
            if ($is_favorite) {
                $this->plugin->vod->remove_favorite_movie($movie_id, $plugin_cookies);
            } else {
                $this->plugin->vod->add_favorite_movie($movie_id, $plugin_cookies);
            }

            $message = $is_favorite ? 'Удалено из Избранного' : 'Добавлено в Избранное';

            return ActionFactory::show_title_dialog($message,
                ActionFactory::invalidate_folders(array(
                    self::get_media_url_str($movie_id),
                    VodFavoritesScreen::get_media_url_str(),
                    )));
        }

        return null;
    }
}

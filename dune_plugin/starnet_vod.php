<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/vod/abstract_vod.php';
require_once 'lib/vod/movie.php';
require_once 'lib/default_dune_plugin.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Vod extends Abstract_Vod
{
    const FAVORITES_LIST = 'favorite_items';
    const HISTORY_LIST = 'history_items';

    /**
     * @var Default_Dune_Plugin
     */
    protected $plugin;

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        $this->plugin = $plugin;
        parent::__construct();
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @throws Exception
     */
    public function try_load_movie($movie_id, &$plugin_cookies)
    {
        $this->set_cached_movie($this->plugin->config->TryLoadMovie($movie_id, $plugin_cookies));
    }

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    /**
     * @param $plugin_cookies
     */
    protected function load_favorites($plugin_cookies)
    {
        $fav_movie_ids = HD::get_items(self::FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name($plugin_cookies), true);
        $this->set_fav_movie_ids($fav_movie_ids);
        hd_print("load_favorites: Movies loaded from favorites: " . count($fav_movie_ids));
    }

    /**
     * @param array $fav_movie_ids
     * @param $plugin_cookies
     */
    protected function do_save_favorite_movies($fav_movie_ids, $plugin_cookies)
    {
        HD::put_items(self::FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name($plugin_cookies), $fav_movie_ids);
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    /**
     * @return array|null
     */
    public function get_vod_list_folder_views()
    {
        return $this->plugin->GET_VOD_MOVIE_LIST_FOLDER_VIEWS();
    }

    /**
     * @return array[]
     */
    public function get_vod_search_folder_views()
    {
        return $this->plugin->GET_TEXT_ONE_COL_VIEWS();
    }

    /**
     * @return bool
     */
    public function is_favorites_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED);
    }

    /**
     * @return bool
     */
    public function is_movie_page_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED);
    }

    protected function do_save_history_movies($history_items, $plugin_cookies)
    {
        HD::put_items(self::HISTORY_LIST . "_" . $this->plugin->config->get_vod_template_name($plugin_cookies), $history_items);
    }

    protected function load_history($plugin_cookies)
    {
        $history_items = HD::get_items(self::HISTORY_LIST . "_" . $this->plugin->config->get_vod_template_name($plugin_cookies), true);
        $this->set_history_items($history_items);
        hd_print("load_history: movies loaded from history: " . count($history_items));
    }
}

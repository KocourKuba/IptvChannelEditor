<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/vod/abstract_vod.php';
require_once 'lib/vod/movie.php';
require_once 'lib/default_dune_plugin.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Vod extends Abstract_Vod
{
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
    protected function load_favorites(&$plugin_cookies)
    {
        $fav_movie_ids = $this->get_fav_movie_ids_from_cookies($plugin_cookies);

        foreach ($fav_movie_ids as $movie_id) {
            if ($this->has_cached_short_movie($movie_id)) {
                continue;
            }

            $this->ensure_movie_loaded($movie_id, $plugin_cookies);
        }

        $this->set_fav_movie_ids($fav_movie_ids);

        $favorites = count($fav_movie_ids);
        hd_print("load_favorites: The $favorites favorite movies loaded.");
    }

    /**
     * @param array $fav_movie_ids
     * @param $plugin_cookies
     */
    protected function do_save_favorite_movies(&$fav_movie_ids, &$plugin_cookies)
    {
        $plugin_cookies->favorite_movies = implode(',', $fav_movie_ids);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_fav_movie_ids_from_cookies($plugin_cookies)
    {
        if (!isset($plugin_cookies->favorite_movies)) {
            return array();
        }

        $arr = explode(",", $plugin_cookies->favorite_movies);

        $ids = array();
        foreach ($arr as $id) {
            if (preg_match('/\S/', $id)) {
                $ids[] = $id;
            }
        }
        return $ids;
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
        return $this->plugin->config->get_feature(VOD_SUPPORTED);
    }

    /**
     * @return bool
     */
    public function is_movie_page_supported()
    {
        return $this->plugin->config->get_feature(VOD_SUPPORTED);
    }
}

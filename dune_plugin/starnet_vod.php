<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/vod/abstract_vod.php';
require_once 'lib/vod/movie.php';

///////////////////////////////////////////////////////////////////////////

class StarnetVod extends AbstractVod
{
    public static $config = null;

    public function __construct()
    {
        parent::__construct();
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function try_load_movie($movie_id, &$plugin_cookies)
    {
        $this->set_cached_movie(self::$config->TryLoadMovie($movie_id, $plugin_cookies));
    }

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    /**
     * @throws Exception
     */
    protected function load_favorites(&$plugin_cookies)
    {
        $fav_movie_ids = $this->get_fav_movie_ids_from_cookies($plugin_cookies);

        foreach ($fav_movie_ids as $movie_id) {
            if ($this->has_cached_short_movie($movie_id))
                continue;

            $this->ensure_movie_loaded($movie_id, $plugin_cookies);
        }

        $this->set_fav_movie_ids($fav_movie_ids);

        $favorites = count($fav_movie_ids);
        if ($favorites > 0)
            hd_print("The $favorites favorite movies loaded.");
    }

    protected function do_save_favorite_movies(&$fav_movie_ids, &$plugin_cookies)
    {
        $this->set_fav_movie_ids_to_cookies($plugin_cookies, $fav_movie_ids);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_fav_movie_ids_from_cookies($plugin_cookies)
    {
        if (!isset($plugin_cookies->favorite_movies))
            return array();

        $arr = preg_split('/,/', $plugin_cookies->favorite_movies);

        $ids = array();
        foreach ($arr as $id) {
            if (preg_match('/\S/', $id))
                $ids[] = $id;
        }
        return $ids;
    }

    public function set_fav_movie_ids_to_cookies(&$plugin_cookies, &$ids)
    {
        $plugin_cookies->favorite_movies = implode(',', $ids);
    }

    public function get_search_media_url_str($pattern)
    {
        return StarnetVodListScreen::get_media_url_str('search', $pattern);
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    public function get_vod_list_folder_views()
    {
        return self::$config->GET_VOD_MOVIE_LIST_FOLDER_VIEWS();
    }

    public function get_vod_search_folder_views()
    {
        return self::$config->GET_TEXT_ONE_COL_VIEWS();
    }

    public function is_favorites_supported()
    {
        $config = self::$config;
        return $config::$VOD_FAVORITES_SUPPORTED;
    }

    public function is_movie_page_supported()
    {
        $config = self::$config;
        return $config::$VOD_MOVIE_PAGE_SUPPORTED;
    }
}

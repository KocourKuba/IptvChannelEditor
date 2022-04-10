<?php
require_once 'vod.php';

abstract class AbstractVod implements Vod
{
    private $short_movie_by_id;

    private $movie_by_id;
    private $failed_movie_ids;

    private $fav_movie_ids;
    private $genres;

    private static $pages = array();
    private static $is_entered = false;
    private static $movie_counter = array();

    protected function __construct()
    {
        $this->short_movie_by_id = array();
        $this->movie_by_id = array();
        $this->failed_movie_ids = array();
    }

    ///////////////////////////////////////////////////////////////////////

    public function set_cached_short_movie(ShortMovie $short_movie)
    {
        $this->short_movie_by_id[$short_movie->id] = $short_movie;
    }

    /**
     * @throws Exception
     */
    public function set_cached_movie(Movie $movie)
    {
        $this->movie_by_id[$movie->id] = $movie;

        $this->set_cached_short_movie(new ShortMovie($movie->id, $movie->name, $movie->poster_url));
    }

    public function set_failed_movie_id($movie_id)
    {
        $this->failed_movie_ids[$movie_id] = true;
    }

    public function set_fav_movie_ids($fav_movie_ids)
    {
        $this->fav_movie_ids = $fav_movie_ids;
    }

    ///////////////////////////////////////////////////////////////////////

    public function is_failed_movie_id($movie_id)
    {
        return isset($this->failed_movie_ids[$movie_id]);
    }

    public function has_cached_movie($movie_id)
    {
        return isset($this->movie_by_id[$movie_id]);
    }

    public function get_cached_movie($movie_id)
    {
        return isset($this->movie_by_id[$movie_id]) ?
            $this->movie_by_id[$movie_id] : null;
    }

    public function has_cached_short_movie($movie_id)
    {
        return isset($this->short_movie_by_id[$movie_id]);
    }

    public function get_cached_short_movie($movie_id)
    {
        return isset($this->short_movie_by_id[$movie_id]) ?
            $this->short_movie_by_id[$movie_id] : null;
    }

    public function clear_movie_cache()
    {
        $this->short_movie_by_id = array();
        $this->movie_by_id = array();
        $this->failed_movie_ids = array();

        $this->fav_movie_ids = null;
    }

    public function clear_genre_cache()
    {
        $this->genres = null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function ensure_movie_loaded($movie_id, &$plugin_cookies)
    {
        if (!isset($movie_id)) {
            throw new Exception('Movie ID is not set');
        }

        if ($this->is_failed_movie_id($movie_id)) {
            hd_print("No movie with ID: $movie_id");
            return null;
        }

        $movie = $this->get_cached_movie($movie_id);
        if ($movie === null) {
            $this->try_load_movie($movie_id, $plugin_cookies);
        }
    }

    /**
     * @throws Exception
     */
    public function get_loaded_movie($movie_id, &$plugin_cookies)
    {
        $this->ensure_movie_loaded($movie_id, $plugin_cookies);

        return $this->get_cached_movie($movie_id);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_vod_info(MediaURL $media_url, &$plugin_cookies)
    {
        $movie = $this->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if ($movie === null) {
            return null;
        }

        $sel_id = isset($media_url->series_id) ?
            $media_url->series_id : null;

        return $movie->get_vod_info($sel_id, (isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * This method should be overridden if and only if the
     * $playback_url_is_stream_url is false.
     * @throws Exception
     */
    public function get_vod_stream_url($playback_url, &$plugin_cookies)
    {
        throw new Exception('Not implemented.');
    }

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    /**
     * This function is responsible for the following:
     *  - $this->fav_movie_ids should be initialized with array of movie
     *  ids
     *  - each of these movie ids should have loaded ShortMovie in
     *  short_movie_by_id map.
     */
    protected function load_favorites(&$plugin_cookies)
    {
        hd_print('AbstractVod: Not implemented');
    }

    // This function should not fail.
    protected function do_save_favorite_movies(&$fav_movie_ids, &$plugin_cookies)
    { /* nop */
    }

    public function ensure_favorites_loaded(&$plugin_cookies)
    {
        if ($this->fav_movie_ids !== null) {
            return;
        }

        $this->load_favorites($plugin_cookies);

        if ($this->fav_movie_ids === null) {
            hd_print('Favorites not loaded.');
        }
    }

    public function get_favorite_movie_ids()
    {
        return $this->fav_movie_ids;
    }

    public function is_favorite_movie_id($movie_id)
    {
        if (!$this->is_favorites_supported()) {
            hd_print("Favorites not supported");
            return false;
        }

        $fav_movie_ids = $this->get_favorite_movie_ids();
        return in_array($movie_id, $fav_movie_ids);
    }

    public function change_vod_favorites($fav_op_type, $movie_id, &$plugin_cookies)
    {
        $fav_movie_ids = $this->get_favorite_movie_ids();

        switch ($fav_op_type) {
            case PLUGIN_FAVORITES_OP_ADD:
                hd_print("Try to add movie id: $movie_id");
                if (!empty($movie_id) && in_array($movie_id, $fav_movie_ids) === false) {
                    hd_print("Success");
                    $fav_movie_ids[] = $movie_id;
                }
                break;
            case PLUGIN_FAVORITES_OP_REMOVE:
                hd_print("Try to remove movie id: $movie_id");
                if(empty($movie_id)) break;

                $k = array_search($movie_id, $fav_movie_ids);
                if ($k !== false) {
                    hd_print("Success");
                    unset ($fav_movie_ids[$k]);
                }
                break;
            case 'clear_favorites':
                $fav_movie_ids = array();
                break;
            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $k = array_search($movie_id, $fav_movie_ids);
                if ($k !== false && $k !== 0) {
                    $t = $fav_movie_ids[$k - 1];
                    $fav_movie_ids[$k - 1] = $fav_movie_ids[$k];
                    $fav_movie_ids[$k] = $t;
                }
                break;
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $k = array_search($movie_id, $fav_movie_ids);
                if ($k !== false && $k !== count($fav_movie_ids) - 1) {
                    $t = $fav_movie_ids[$k + 1];
                    $fav_movie_ids[$k + 1] = $fav_movie_ids[$k];
                    $fav_movie_ids[$k] = $t;
                }
                break;
        }

        $this->set_fav_movie_ids($fav_movie_ids);
        $this->do_save_favorite_movies($fav_movie_ids, $plugin_cookies);

        return ActionFactory::invalidate_folders(array(VodFavoritesScreen::get_media_url_str()));
    }

    ///////////////////////////////////////////////////////////////////////
    // Genres.

    /**
     * @throws Exception
     */
    protected function load_genres(&$plugin_cookies)
    {
        hd_print("AbstractVod::load_genres: Not implemented.");
        return null;
    }

    /**
     * @throws Exception
     */
    public function get_genre_icon_url($genre_id)
    {
        throw new Exception('Not implemented');
    }

    /**
     * @throws Exception
     */
    public function get_genre_media_url_str($genre_id)
    {
        throw new Exception('Not implemented');
    }

    /**
     * @throws Exception
     */
    public function ensure_genres_loaded(&$plugin_cookies)
    {
        if ($this->genres !== null) {
            return;
        }

        $this->genres = $this->load_genres($plugin_cookies);

        if ($this->genres === null) {
            throw new Exception('Invalid VOD genres loaded');
        }
    }

    /**
     * @throws Exception
     */
    public function get_genre_ids()
    {
        if ($this->genres === null) {
            throw new Exception('VOD genres not loaded');
        }

        return array_keys($this->genres);
    }

    /**
     * @throws Exception
     */
    public function get_genre_caption($genre_id)
    {
        if ($this->genres === null) {
            throw new Exception('VOD genres not loaded');
        }

        return $this->genres[$genre_id];
    }

    ///////////////////////////////////////////////////////////////////////
    // Search.

    /**
     * @throws Exception
     */
    public function get_search_media_url_str($pattern)
    {
        throw new Exception('Not implemented');
    }

    ///////////////////////////////////////////////////////////////////////
    // Filter.

    /**
     * @throws Exception
     */
    public function get_filter_media_url_str($pattern)
    {
        throw new Exception('Not implemented');
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    /**
     * @throws Exception
     */
    public function get_vod_list_folder_views()
    {
        throw new Exception('Not implemented');
    }

    /**
     * @throws Exception
     */
    public function get_vod_genres_folder_views()
    {
        throw new Exception('Not implemented');
    }

    ///////////////////////////////////////////////////////////////////////
    // Archive.

    public function get_archive(MediaURL $media_url)
    {
        return null;
    }

    ///////////////////////////////////////////////////////////////////////
    // Hook.

    public function folder_entered(MediaURL $media_url, &$plugin_cookies)
    { /* Nop */
    }
}

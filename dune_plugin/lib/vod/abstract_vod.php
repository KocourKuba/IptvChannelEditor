<?php
require_once 'vod.php';
require_once 'short_movie.php';

abstract class Abstract_Vod implements Vod
{
    /**
     * @var array|Short_Movie[]
     */
    private $short_movie_by_id = array();

    /**
     * @var array|Movie[]
     */
    private $movie_by_id = array();

    /**
     * @var array|bool[]
     */
    private $failed_movie_ids = array();

    /**
     * @var array
     */
    private $fav_movie_ids;

    /**
     * @var array
     */
    private $history_items;

    /**
     * @var array
     */
    private $genres;

    protected function __construct()
    {
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Short_Movie $short_movie
     */
    public function set_cached_short_movie(Short_Movie $short_movie)
    {
        $this->short_movie_by_id[$short_movie->id] = $short_movie;
    }

    /**
     * @param Movie $movie
     */
    public function set_cached_movie(Movie $movie)
    {
        $this->movie_by_id[$movie->id] = $movie;
        $this->set_cached_short_movie(new Short_Movie($movie->id, $movie->name, $movie->poster_url));
    }

    /**
     * @param string $movie_id
     */
    public function set_failed_movie_id($movie_id)
    {
        $this->failed_movie_ids[$movie_id] = true;
    }

    /**
     * @param array $fav_movie_ids
     */
    public function set_fav_movie_ids($fav_movie_ids, $plugin_cookies = null)
    {
        $this->fav_movie_ids = $fav_movie_ids;
        if (!is_null($plugin_cookies)) {
            $this->do_save_favorite_movies($fav_movie_ids, $plugin_cookies);
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $movie_id
     * @return bool
     */
    public function is_failed_movie_id($movie_id)
    {
        return isset($this->failed_movie_ids[$movie_id]);
    }

    /**
     * @param string $movie_id
     * @return bool
     */
    public function has_cached_movie($movie_id)
    {
        return isset($this->movie_by_id[$movie_id]);
    }

    /**
     * @param string $movie_id
     * @return Movie|null
     */
    public function get_cached_movie($movie_id)
    {
        return isset($this->movie_by_id[$movie_id]) ? $this->movie_by_id[$movie_id] : null;
    }

    /**
     * @param string $movie_id
     * @return bool
     */
    public function has_cached_short_movie($movie_id)
    {
        return isset($this->short_movie_by_id[$movie_id]);
    }

    /**
     * @param string $movie_id
     * @return Short_Movie|null
     */
    public function get_cached_short_movie($movie_id)
    {
        return isset($this->short_movie_by_id[$movie_id]) ? $this->short_movie_by_id[$movie_id] : null;
    }

    public function clear_movie_cache()
    {
        hd_print(__METHOD__ . ": Abstract_Vod::clear_movie_cache: movie cache cleared");

        $this->short_movie_by_id = array();
        $this->movie_by_id = array();
        $this->failed_movie_ids = array();
        unset($this->fav_movie_ids, $this->history_items);
    }

    public function clear_genre_cache()
    {
        unset($this->genres);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     */
    public function ensure_movie_loaded($movie_id, &$plugin_cookies)
    {
        if (!isset($movie_id)) {
            hd_print(__METHOD__ . ": Movie ID is not set");
            return;
        }

        if ($this->is_failed_movie_id($movie_id)) {
            hd_print(__METHOD__ . ": No movie with ID: $movie_id");
            return;
        }

        $movie = $this->get_cached_movie($movie_id);
        if ($movie === null) {
            hd_print(__METHOD__ . ": Movie $movie_id not in cache. Load info.");
            $this->try_load_movie($movie_id, $plugin_cookies);
            hd_print(__METHOD__ . ": Movie $movie_id loaded");
        } else {
            hd_print(__METHOD__ . ": Movie $movie_id is in cache.");
        }
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie|null
     */
    public function get_loaded_movie($movie_id, &$plugin_cookies)
    {
        $this->ensure_movie_loaded($movie_id, $plugin_cookies);

        return $this->get_cached_movie($movie_id);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_vod_info(MediaURL $media_url, &$plugin_cookies)
    {
        $movie = $this->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if ($movie === null) {
            return null;
        }

        return $movie->get_movie_play_info($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * This method should be overridden if and only if the
     * $playback_url_is_stream_url is false.
     * @param string $playback_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_stream_url($playback_url, &$plugin_cookies)
    {
        return $playback_url;
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
    abstract protected function load_favorites($plugin_cookies);

    // This function should not fail.
    abstract protected function do_save_favorite_movies($fav_movie_ids, $plugin_cookies);

    public function ensure_favorites_loaded($plugin_cookies)
    {
        if (!is_null($this->fav_movie_ids)) {
            return;
        }

        $this->load_favorites($plugin_cookies);
    }

    /**
     * @return array
     */
    public function get_favorite_movie_ids()
    {
        return $this->fav_movie_ids;
    }

    /**
     * @param string $movie_id
     * @return bool
     */
    public function is_favorite_movie_id($movie_id)
    {
        if (!$this->is_favorites_supported()) {
            hd_print(__METHOD__ . ": Favorites not supported");
            return false;
        }

        $fav_movie_ids = $this->get_favorite_movie_ids();
        return !is_null($fav_movie_ids) && in_array($movie_id, $fav_movie_ids);
    }

    /**
     * @param string $fav_op_type
     * @param string $movie_id
     * @param $plugin_cookies
     * @return array
     */
    public function change_vod_favorites($fav_op_type, $movie_id, &$plugin_cookies)
    {
        //hd_print("change_vod_favorites: $fav_op_type, $movie_id");
        $this->ensure_favorites_loaded($plugin_cookies);
        $fav_movie_ids = $this->get_favorite_movie_ids();

        switch ($fav_op_type) {
            case PLUGIN_FAVORITES_OP_ADD:
                hd_print(__METHOD__ . ": Try to add movie id: $movie_id");
                if (!is_null($movie_id) && in_array($movie_id, $fav_movie_ids) === false) {
                    hd_print(__METHOD__ . ": Movie id: $movie_id added to favorites");
                    $fav_movie_ids[] = $movie_id;
                }
                break;
            case PLUGIN_FAVORITES_OP_REMOVE:
                hd_print(__METHOD__ . ": Try to remove movie id: $movie_id");
                $k = array_search($movie_id, $fav_movie_ids);
                if ($k !== false) {
                    hd_print(__METHOD__ . ": Movie id: $movie_id removed from favorites");
                    unset ($fav_movie_ids[$k]);
                }
                break;
            case ACTION_CLEAR_FAVORITES:
                hd_print(__METHOD__ . ": Movie favorites cleared");
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

        $this->set_fav_movie_ids($fav_movie_ids, $plugin_cookies);

        return Action_Factory::invalidate_folders(array(Starnet_Vod_Favorites_Screen::get_media_url_str()));
    }

    ///////////////////////////////////////////////////////////////////////
    // History.

    abstract protected function load_history($plugin_cookies);

    abstract protected function do_save_history_movies($history_items, $plugin_cookies);

    /**
     * @return array
     */
    public function get_history_movies()
    {
        return $this->history_items;
    }

    /**
     * @param array $history_items
     */
    public function set_history_items($history_items, $plugin_cookies = null)
    {
        $this->history_items = $history_items;
        if (!is_null($plugin_cookies))
            $this->do_save_history_movies($history_items, $plugin_cookies);
    }

    public function ensure_history_loaded($plugin_cookies)
    {
        if (!is_null($this->history_items)) {
            return;
        }

        $this->load_history($plugin_cookies);
    }

    public function add_movie_to_history($id, $series_idx, $item, &$plugin_cookies)
    {
        //hd_print(__METHOD__ . ": add movie to history: id: $id, series: $series_idx");
        $this->ensure_history_loaded($plugin_cookies);

        if (isset($this->history_items[$id])) {
            HD::array_unshift_assoc($this->history_items, $id, $this->history_items[$id]);
        }

        $this->history_items[$id][$series_idx] = $item;

        if (count($this->history_items) === 64) {
            array_pop($this->history_items);
        }

        $this->set_history_items($this->history_items, $plugin_cookies);
    }

    public function remove_movie_from_history($id, $plugin_cookies)
    {
        $this->ensure_history_loaded($plugin_cookies);

        unset($this->history_items[$id]);

        $this->set_history_items($this->history_items, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    // Genres.

    /**
     * @param $plugin_cookies
     * @return array|null
     */
    protected function load_genres($plugin_cookies)
    {
        hd_print(__METHOD__ . ": Not implemented.");
        return null;
    }

    /**
     * @param string $genre_id
     * @return string|null
     */
    public function get_genre_icon_url($genre_id)
    {
        hd_print(__METHOD__ . ": Not implemented.");
        return null;
    }

    /**
     * @param string $genre_id
     * @return string|null
     */
    public function get_genre_media_url_str($genre_id)
    {
        hd_print(__METHOD__ . ": Not implemented.");
        return null;
    }

    public function ensure_genres_loaded($plugin_cookies)
    {
        if ($this->genres !== null) {
            return;
        }

        $this->genres = $this->load_genres($plugin_cookies);

        if ($this->genres === null) {
            hd_print(__METHOD__ . ": Not implemented.");
        }
    }

    /**
     * @return string[]|null
     */
    public function get_genre_ids()
    {
        if ($this->genres === null) {
            hd_print(__METHOD__ . ": Not implemented.");
            return null;
        }

        return array_keys($this->genres);
    }

    /**
     * @param string $genre_id
     * @return string|null
     */
    public function get_genre_caption($genre_id)
    {
        if ($this->genres === null) {
            hd_print(__METHOD__ . ": Not implemented.");
            return null;
        }

        return $this->genres[$genre_id];
    }

    ///////////////////////////////////////////////////////////////////////
    // Search.

    ///////////////////////////////////////////////////////////////////////
    // Filter.

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    /**
     * @return array|null
     */
    public function get_vod_list_folder_views()
    {
        hd_print(__METHOD__ . ": Not implemented.");
        return null;
    }

    /**
     * @return array|null
     */
    public function get_vod_genres_folder_views()
    {
        hd_print(__METHOD__ . ": Not implemented.");
        return null;
    }
}

<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/abstract_vod.php';
require_once 'lib/movie.php';
require_once 'lib/default_dune_plugin.php';

///////////////////////////////////////////////////////////////////////////

class Starnet_Vod extends Abstract_Vod
{
    const VOD_FAVORITES_LIST = 'vod_favorite_items';
    const VOD_HISTORY_ITEMS = 'vod_history_items';

    /**
     * @var Default_Dune_Plugin
     */
    protected $plugin;

    /**
     * @var Ordered_Array
     */
    protected $fav_movie_ids;

    /**
     * @var array
     */
    protected $history_items;

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        $this->plugin = $plugin;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @inheritDoc
     * @throws Exception
     */
    public function try_load_movie($movie_id)
    {
        $this->set_cached_movie($this->plugin->config->TryLoadMovie($movie_id));
    }

    /**
     * @inheritDoc
     */
    public function clear_movie_cache()
    {
        parent::clear_movie_cache();

        $this->fav_movie_ids = null;
        $this->history_items = null;
    }

    /**
     * @return bool
     */
    public function is_movie_page_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED);
    }

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    /**
     * @return Ordered_Array
     */
    public function get_favorite_movie_ids()
    {
        if ($this->fav_movie_ids === null) {
            $this->load_favorites();
        }

        return $this->fav_movie_ids;
    }

    /**
     * @param Ordered_Array $fav_movie_ids
     */
    public function set_movie_favorites($fav_movie_ids)
    {
        $this->fav_movie_ids = $fav_movie_ids;
        $this->save_movie_favorites();
    }

    /**
     * @param string $movie_id
     * @return bool
     */
    public function is_favorite_movie_id($movie_id)
    {
        if (!$this->is_favorites_supported()) {
            hd_debug_print("Favorites not supported");
            return false;
        }

        $fav_movie_ids = $this->get_favorite_movie_ids();
        return !is_null($fav_movie_ids) && $fav_movie_ids->in_order($movie_id);
    }

    /**
     * This function is responsible for the following:
     *  - $this->fav_movie_ids should be initialized with array of movie
     *  ids
     *  - each of these movie ids should have loaded ShortMovie in
     *  short_movie_by_id map.
     *
     */
    protected function load_favorites()
    {
        $fav_movie_ids = HD::get_data_items(self::VOD_FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name());
        $this->fav_movie_ids = new Ordered_Array($fav_movie_ids);
        hd_debug_print("Movies loaded from favorites: " . count($fav_movie_ids));
    }

    /**
     * This function should not fail.
     * @return void
     */
    public function save_movie_favorites()
    {
        $path = self::VOD_FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name();
        $fav_movie_ids = $this->get_favorite_movie_ids();
        if ($fav_movie_ids->size() === 0) {
            HD::erase_data_items($path);
        } else {
            HD::put_data_items($path, $fav_movie_ids->get_order());
        }
    }

    /**
     * @param string $fav_op_type
     * @param string $movie_id
     * @return array
     */
    public function change_vod_favorites($fav_op_type, $movie_id)
    {
        hd_debug_print(null, true);

        switch ($fav_op_type) {
            case PLUGIN_FAVORITES_OP_ADD:
                if ($this->get_favorite_movie_ids()->add_item($movie_id)) {
                    hd_debug_print("Movie id: $movie_id added to favorites");
                }
                break;

            case PLUGIN_FAVORITES_OP_REMOVE:
                if ($this->get_favorite_movie_ids()->remove_item($movie_id)) {
                    hd_debug_print("Movie id: $movie_id removed from favorites");
                }
                break;

            case ACTION_ITEMS_CLEAR:
                hd_debug_print("Movie favorites cleared");
                $this->get_favorite_movie_ids()->clear();
                break;

            case PLUGIN_FAVORITES_OP_MOVE_UP:
                $this->get_favorite_movie_ids()->arrange_item($movie_id, Ordered_Array::UP);
                break;

            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                $this->get_favorite_movie_ids()->arrange_item($movie_id, Ordered_Array::DOWN);
                break;
        }

        return Action_Factory::invalidate_folders(array(Starnet_Vod_Favorites_Screen::ID));
    }

    /**
     * @return bool
     */
    public function is_favorites_supported()
    {
        return $this->plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED);
    }

    ///////////////////////////////////////////////////////////////////////
    // History.

    protected function do_save_history_movies()
    {
        $path = $this->plugin->get_history_path(self::VOD_HISTORY_ITEMS . "_" . $this->plugin->config->get_vod_template_name());
        $history_items = $this->get_history_movies();
        if (empty($history_items)) {
            HD::erase_items($path);
        } else {
            HD::put_items($path, $history_items);
        }
    }

    protected function load_history()
    {
        $filename = self::VOD_HISTORY_ITEMS . "_" . $this->plugin->config->get_vod_template_name();
        $this->history_items = HD::get_items($this->plugin->get_history_path($filename));
        hd_debug_print("movies loaded from history: " . count($this->history_items));
    }

    /**
     * @return array
     */
    public function get_history_movies()
    {
        if ($this->history_items === null) {
            $this->load_history();
        }


        return $this->history_items;
    }

    /**
     * @param array $history_items
     */
    public function set_history_movies($history_items)
    {
        $this->history_items = $history_items;
        $this->do_save_history_movies();
    }

    public function add_movie_to_history($id, $series_idx, $item)
    {
        hd_debug_print("add movie to history: id: $id, series: $series_idx", true);

        $history_items = $this->get_history_movies();
        if (isset($history_items[$id])) {
            HD::array_unshift_assoc($history_items, $id, $history_items[$id]);
        }

        $history_items[$id][$series_idx] = $item;

        if (count($history_items) === 64) {
            array_pop($history_items);
        }

        $this->set_history_movies($history_items);
    }

    public function remove_movie_from_history($id)
    {
        $history_items = $this->get_history_movies();
        unset($history_items[$id]);

        $this->set_history_movies($history_items);
    }
}

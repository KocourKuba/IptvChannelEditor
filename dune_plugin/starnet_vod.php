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

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    /**
     * @inheritDoc
     */
    protected function load_favorites()
    {
        $fav_movie_ids = HD::get_data_items(self::VOD_FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name());
        $this->set_fav_movie_ids($fav_movie_ids);
        hd_debug_print("Movies loaded from favorites: " . count($fav_movie_ids));
    }

    /**
     * @inheritDoc
     */
    protected function do_save_favorite_movies($fav_movie_ids)
    {
        HD::put_data_items(self::VOD_FAVORITES_LIST . "_" . $this->plugin->config->get_vod_template_name(), $fav_movie_ids);
    }

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

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

    protected function do_save_history_movies($history_items)
    {
        $filename = self::VOD_HISTORY_ITEMS . "_" . $this->plugin->config->get_vod_template_name();
        $path = $this->plugin->get_history_path($filename);
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
}

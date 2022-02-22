<?php

require_once 'lib/vod/vod_list_screen.php';

class StarnetVodListScreen extends VodListScreen
{
    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct($plugin);
    }

    public static function get_media_url_str($cat_id, $genre_id, $name = false)
    {
        $arr['screen_id'] = self::ID;
        $arr['category_id'] = $cat_id;
        $arr['genre_id'] = $genre_id;
        if ($name !== false) {
            $arr['name'] = $name;
        }

        return MediaURL::encode($arr);
    }

    /**
     * @throws Exception
     */
    protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        hd_print("get_short_movie_range: '$media_url->category_id', from_idx: $from_ndx");
        $this->plugin->config->try_reset_pages();
        $key = $media_url->category_id . "_" . $media_url->genre_id;

        $movies = array();
        if ($this->plugin->config->get_movie_counter($key) <= 0 || $this->plugin->config->is_lazy_load_vod()) {
            if ($media_url->category_id === 'search') {
                $movies = $this->plugin->config->getSearchList($media_url->genre_id, $plugin_cookies);
            } else if ($media_url->category_id === 'filter') {
                $movies = $this->plugin->config->getFilterList($media_url->genre_id, $plugin_cookies);
            } else if ($media_url->category_id === 'all' || empty($media_url->genre_id)) {
                $movies = $this->plugin->config->getVideoList($media_url->category_id, $plugin_cookies);
            } else {
                $movies = $this->plugin->config->getVideoList($key, $plugin_cookies);
            }
        }

        $count = count($movies);
        if ($count) {
            $this->plugin->config->add_movie_counter($key, $count);
            return new ShortMovieRange($from_ndx, $this->plugin->config->get_movie_counter($key), $movies);
        }

        return new ShortMovieRange(0, 0);
    }
}

<?php

require_once 'lib/vod/vod_list_screen.php';
require_once 'lib/vod/short_movie_range.php';

class Starnet_Vod_List_Screen extends Vod_List_Screen
{
    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct($plugin);
    }

    /**
     * @param string $category_id
     * @param string $genre_id
     * @param string $name
     * @return false|string
     */
    public static function get_media_url_str($category_id, $genre_id, $name = false)
    {
        $arr['screen_id'] = self::ID;
        $arr['category_id'] = $category_id;
        $arr['genre_id'] = $genre_id;
        if ($name !== false) {
            $arr['name'] = $name;
        }

        return MediaURL::encode($arr);
    }

    /**
     * @param MediaURL $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return Short_Movie_Range
     */
    protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        hd_print("get_short_movie_range: '$media_url->category_id', from_idx: $from_ndx");
        $this->plugin->config->try_reset_pages();
        $key = $media_url->category_id . "_" . $media_url->genre_id;

        $movies = array();
        if ($this->plugin->config->get_movie_counter($key) <= 0 || $this->plugin->config->get_feature(VOD_LAZY_LOAD)) {
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
            return new Short_Movie_Range($from_ndx, $this->plugin->config->get_movie_counter($key), $movies);
        }

        return new Short_Movie_Range(0, 0);
    }
}

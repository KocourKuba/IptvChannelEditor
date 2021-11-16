<?php

require_once 'lib/vod/vod_list_screen.php';

class StarnetVodListScreen extends VodListScreen
{
    public function __construct(Vod $vod)
    {
        parent::__construct($vod);
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
        static::$config->try_reset_pages();
        $key = $media_url->category_id . "_" . $media_url->genre_id;

        $movies = array();
        if (static::$config->get_movie_counter($key) <= 0 || static::$config->is_lazy_load_vod()) {
            if ($media_url->category_id === 'search') {
                    $movies = static::$config->getSearchList($media_url->genre_id, $plugin_cookies);
            } else if ($media_url->category_id === 'all') {
                $movies = static::$config->getVideoList($media_url->category_id, $plugin_cookies);
            } else {
                $movies = static::$config->getVideoList($media_url->category_id . "_" . $media_url->genre_id, $plugin_cookies);
            }
        }

        $count = count($movies);
        if ($count) {
            static::$config->add_movie_counter($key, $count);
            return new ShortMovieRange($from_ndx, static::$config->get_movie_counter($key), $movies);
        }

        return new ShortMovieRange(0, 0);
    }
}

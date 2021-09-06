<?php

require_once 'lib/vod/vod_list_screen.php';

class StarnetVodListScreen extends VodListScreen
{
    public static $config = null;

    public function __construct(Vod $vod)
    {
        parent::__construct($vod);
    }

    public static function get_media_url_str($cat_id, $genre_id, $name = false)
    {
        $arr['screen_id'] = self::ID;
        $arr['category_id'] = $cat_id;
        $arr['genre_id'] = $genre_id;
        if ($name == true)
            $arr['name'] = $name;

        return MediaURL::encode($arr);
    }

    /**
     * @throws Exception
     */
    protected function get_short_movie_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        if ($media_url->category_id == 'search') {
            $movies = self::$config->getSearchList($media_url->genre_id);
        } else {
            $movies = self::$config->getVideoList($media_url->category_id, $media_url->genre_id);
        }

        if (count($movies))
            return new ShortMovieRange($from_ndx, count($movies), $movies);

        return new ShortMovieRange(0, 0);
    }
}

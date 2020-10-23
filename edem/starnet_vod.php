<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/vod/abstract_vod.php';
require_once 'lib/vod/movie.php';

///////////////////////////////////////////////////////////////////////////

class DemoVod extends AbstractVod
{
    public function __construct()
    {
        parent::__construct(
            DemoConfig::VOD_FAVORITES_SUPPORTED,
            DemoConfig::VOD_MOVIE_PAGE_SUPPORTED,
            true);
    }

    ///////////////////////////////////////////////////////////////////////

    public function try_load_movie($movie_id, &$plugin_cookies)
    {
        $doc =
            HD::http_get_document(
                sprintf(
                    DemoConfig::MOVIE_INFO_URL_FORMAT,
                    $movie_id));
     
        if (is_null($doc))
            throw new Exception('Can not fetch movie info');

        $xml = simplexml_load_string($doc);

        if ($xml === false)
        {
            hd_print("Error: can not parse XML document.");
//            hd_print("XML-text:\n$doc\n");
            throw new Exception('Illegal XML document');
        }

        if ($xml->getName() !== 'movie_info')
        {
            hd_print("Error: unexpected node '" . $xml->getName() .
                "'. Expected: 'movie_info'");
            throw new Exception('Invalid XML document');
        }

        $movie = new Movie($xml->id);

        $movie->set_data(
            $xml->caption,
            $xml->caption_original,
            $xml->description,
            $xml->poster_url,
            $xml->length,
            $xml->year,
            $xml->director,
            $xml->scenario,
            $xml->actors,
            $xml->genres,
            $xml->rate_imdb,
            $xml->rate_kinopoisk,
            $xml->rate_mpaa,
            $xml->country,
            $xml->budget);

        foreach ($xml->series->item as $item)
        {
            $movie->add_series_data(
                $item->id,
                $item->title,
                $item->playback_url,
                true);
        }

        $this->set_cached_movie($movie);
    }

    ///////////////////////////////////////////////////////////////////////
    // Favorites.

    protected function load_favorites(&$plugin_cookies)
    {
        $fav_movie_ids = $this->get_fav_movie_ids_from_cookies($plugin_cookies);

        foreach ($fav_movie_ids as $movie_id)
        {
            if ($this->has_cached_short_movie($movie_id))
                continue;

            $this->ensure_movie_loaded($movie_id, $plugin_cookies);
        }

        $this->set_fav_movie_ids($fav_movie_ids);

        hd_print('The ' . count($fav_movie_ids) . ' favorite movies loaded.');
    }

    protected function do_save_favorite_movies(&$fav_movie_ids, &$plugin_cookies)
    {
        $this->set_fav_movie_ids_to_cookies($plugin_cookies, $fav_movie_ids);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_fav_movie_ids_from_cookies(&$plugin_cookies)
    {
        if (!isset($plugin_cookies->{'favorite_movies'}))
            return array();

        $arr = preg_split('/,/', $plugin_cookies->{'favorite_movies'});

        $ids = array();
        foreach ($arr as $id)
        {
            if (preg_match('/\S/', $id))
                $ids[] = $id;
        }
        return $ids;
    }

    public function set_fav_movie_ids_to_cookies(&$plugin_cookies, &$ids)
    {
        $plugin_cookies->{'favorite_movies'} = join(',', $ids);
    }

    ///////////////////////////////////////////////////////////////////////
    // Genres.

/*
    protected function load_genres(&$plugin_cookies)
    {
        $doc = $this->session->api_vod_genres();

        $genres = array();
        foreach ($doc->genres as $genre)
            $genres[$genre->id] = $genre->name;

        return $genres;
    }

    public function get_genre_icon_url($genre_id)
    {
        return $this->session->get_icon('mov_genre_default.png');
    }

    public function get_genre_media_url_str($genre_id)
    {
        return DemoVodListScreen::get_media_url_str('genres', $genre_id);
    }
*/

    ///////////////////////////////////////////////////////////////////////
    // Search.

/*
    public function get_search_media_url_str($pattern)
    {
        return DemoVodListScreen::get_media_url_str('search', $pattern);
    }
*/

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    public function get_vod_list_folder_views()
    {
        return DemoConfig::GET_VOD_MOVIE_LIST_FOLDER_VIEWS();
    }
}

///////////////////////////////////////////////////////////////////////////
?>

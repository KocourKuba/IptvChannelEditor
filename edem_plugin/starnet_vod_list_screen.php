<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/vod/vod_list_screen.php';

class DemoVodListScreen extends VodListScreen
{
    public static function get_media_url_str($cat_id)
    {
        $arr['screen_id'] = self::ID;
        $arr['category_id'] = $cat_id;
        return MediaURL::encode($arr);
    }

    ///////////////////////////////////////////////////////////////////////

    public function __construct(Vod $vod)
    {
        parent::__construct($vod);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    protected function get_short_movie_range(MediaURL $media_url, $from_ndx,
                                             &$plugin_cookies)
    {
        $doc =
            HD::http_get_document(
                sprintf(
                    PluginConfig::MOVIE_LIST_URL_FORMAT,
                    $media_url->category_id));

        if (is_null($doc))
            throw new Exception('Can not fetch movie list');

        $xml = simplexml_load_string($doc);

        if ($xml === false) {
            hd_print("Error: can not parse XML document.");
            hd_print("XML-text: $doc.");
            throw new Exception('Illegal XML document');
        }

        if ($xml->getName() !== 'movies') {
            hd_print("Error: unexpected node '" . $xml->getName() . "'. Expected: 'vod_categories'");
            throw new Exception('Invalid XML document');
        }

        $movies = array();

        foreach ($xml->children() as $movie) {
            $movies[] = new ShortMovie(
                strval($movie->id),
                strval($movie->caption),
                strval($movie->poster_url));
        }

        return new ShortMovieRange(0, count($movies), $movies);
    }
}

///////////////////////////////////////////////////////////////////////////
?>

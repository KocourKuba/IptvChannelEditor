<?php

require_once 'movie_series.php';
require_once 'movie_season.php';
require_once 'movie_variant.php';

class Movie
{
    /**
     * @var string
     */
    public $id;

    /**
     * @var string
     */
    public $name = '';

    /**
     * @var string
     */
    public $name_original = '';

    /**
     * @var string
     */
    public $description = '';

    /**
     * @var string
     */
    public $poster_url = '';

    /**
     * @var int
     */
    public $length_min = -1;

    /**
     * @var int
     */
    public $year = 0;

    /**
     * @var string
     */
    public $directors_str = '';

    /**
     * @var string
     */
    public $scenarios_str = '';

    /**
     * @var string
     */
    public $actors_str = '';

    /**
     * @var string
     */
    public $genres_str = '';

    /**
     * @var string
     */
    public $rate_imdb = '';

    /**
     * @var string
     */
    public $rate_kinopoisk = '';

    /**
     * @var string
     */
    public $rate_mpaa = '';

    /**
     * @var string
     */
    public $country = '';

    /**
     * @var string
     */
    public $budget = '';

    /**
     * @var string
     */
    public $type = 'movie';

    /**
     * @var array|Movie_Season[]
     */
    public $season_list;

    /**
     * @var array|Movie_Series[]
     */
    public $series_list;

    /**
     * @var array|string[]
     */
    public $variants_list;

    /**
     * @param string $id
     * @throws Exception
     */
    public function __construct($id)
    {
        if (is_null($id)) {
            HD::print_backtrace();
            throw new Exception("Movie::id is null");
        }

        $this->id = (string)$id;
    }

    /**
     * @param $v
     * @return string
     */
    private function to_string($v)
    {
        return $v === null ? '' : (string)$v;
    }

    /**
     * @param $v
     * @param $default_value
     * @return int
     */
    private function to_int($v, $default_value)
    {
        $v = (string)$v;
        if (!is_numeric($v)) {
            return $default_value;
        }
        $v = (int)$v;
        return $v <= 0 ? $default_value : $v;
    }

    /**
     * @param string $name
     * @param string $name_original
     * @param string $description
     * @param string $poster_url
     * @param string $length_min
     * @param int $year
     * @param string $directors_str
     * @param string $scenarios_str
     * @param string $actors_str
     * @param string $genres_str
     * @param string $rate_imdb
     * @param string $rate_kinopoisk
     * @param string $rate_mpaa
     * @param string $country
     * @param string $budget
     */
    public function set_data(
        $name,
        $name_original,
        $description,
        $poster_url,
        $length_min,
        $year,
        $directors_str,
        $scenarios_str,
        $actors_str,
        $genres_str,
        $rate_imdb,
        $rate_kinopoisk,
        $rate_mpaa,
        $country,
        $budget)
    {
        $this->name = $this->to_string($name);
        $this->name_original = $this->to_string($name_original);
        $this->description = $this->to_string($description);
        $this->poster_url = $this->to_string($poster_url);
        $this->length_min = $this->to_int($length_min, -1);
        $this->year = $this->to_int($year, -1);
        $this->directors_str = $this->to_string($directors_str);
        $this->scenarios_str = $this->to_string($scenarios_str);
        $this->actors_str = $this->to_string($actors_str);
        $this->genres_str = $this->to_string($genres_str);
        $this->rate_imdb = $this->to_string($rate_imdb);
        $this->rate_kinopoisk = $this->to_string($rate_kinopoisk);
        $this->rate_mpaa = $this->to_string($rate_mpaa);
        $this->country = $this->to_string($country);
        $this->budget = $this->to_string($budget);
    }

    /**
     * @param string $id
     * @param string $name
     * @param string $season_url
     * @throws Exception
     */
    public function add_season_data($id, $name, $season_url)
    {
        $season = new Movie_Season($id);
        $season->name = $this->to_string($name);
        $season->season_url = $this->to_string($season_url);
        $this->season_list[] = $season;
    }

    /**
     * @param string $id
     * @param string $name
     * @param string $description
     * @param string $playback_url
     * @param bool $playback_url_is_stream_url
     * @throws Exception
     */
    public function add_series_data($id, $name, $description, $playback_url, $season_id = '', $playback_url_is_stream_url = true)
    {
        $series = new Movie_Series($id);
        $series->name = $this->to_string($name);
        $series->series_desc = $this->to_string($description);
        $series->season_id = $this->to_string($season_id);
        $series->playback_url = $this->to_string($playback_url);
        $series->playback_url_is_stream_url = $playback_url_is_stream_url;
        $this->series_list[$id] = $series;
    }

    /**
     * @param $id
     * @param $name
     * @param $description
     * @param $playback_url
     * @param $variants
     * @param $season_id
     * @param $playback_url_is_stream_url
     * @throws Exception
     */
    public function add_series_variants_data($id, $name, $description, $variants, $playback_url = '', $season_id = '', $playback_url_is_stream_url = true)
    {
        $series = new Movie_Series($id);
        $series->name = $this->to_string($name);
        $series->series_desc = $this->to_string($description);
        $series->variants = $variants;
        $series->season_id = $this->to_string($season_id);
        $series->playback_url = $this->to_string($playback_url);
        $series->playback_url_is_stream_url = $playback_url_is_stream_url;
        $this->series_list[$id] = $series;
        $this->variants_list = array_keys($variants);
    }

    /**
     * @return array
     */
    public function get_movie_array()
    {
        return array(
            PluginMovie::name => $this->name,
            PluginMovie::name_original => $this->name_original,
            PluginMovie::description => $this->description,
            PluginMovie::poster_url => $this->poster_url,
            PluginMovie::length_min => $this->length_min,
            PluginMovie::year => $this->year,
            PluginMovie::directors_str => $this->directors_str,
            PluginMovie::scenarios_str => $this->scenarios_str,
            PluginMovie::actors_str => $this->actors_str,
            PluginMovie::genres_str => $this->genres_str,
            PluginMovie::rate_imdb => $this->rate_imdb,
            PluginMovie::rate_kinopoisk => $this->rate_kinopoisk,
            PluginMovie::rate_mpaa => $this->rate_mpaa,
            PluginMovie::country => $this->country,
            PluginMovie::budget => $this->budget
        );
    }

    public function has_variants()
    {
        if (empty($this->series_list)) {
            return false;
        }

        $values = array_values($this->series_list);
        $val = $values[0];
        return isset($val->variants) && count($val->variants) > 1;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_movie_play_info($media_url, &$plugin_cookies)
    {
        if (!isset($media_url->screen_id)) {
            HD::print_backtrace();
            throw new Exception("List screen in media url not set: " . $media_url->get_raw_string());
        }

        //hd_print("get_vod_info: selected screen: " . $media_url->get_raw_string());

        switch ($media_url->screen_id) {
            case Vod_Seasons_List_Screen::ID:
                if (!is_array($this->season_list) || count($this->season_list) === 0) {
                    HD::print_backtrace();
                    throw new Exception("Invalid movie: season list is empty");
                }
                $list = $this->series_list;
                break;
            case Vod_Series_List_Screen::ID:
                if (!is_array($this->series_list) || count($this->series_list) === 0) {
                    HD::print_backtrace();
                    throw new Exception("Invalid movie: series list is empty");
                }
                $list = $this->series_list;
                break;
            case Vod_Movie_Screen::ID:
                $list = $this->series_list;
                break;
            default:
                HD::print_backtrace();
                throw new Exception("Unknown list screen: $media_url->screen_id");
        }

        $sel_id = isset($media_url->series_id) ? $media_url->series_id : null;
        //hd_print("selected id: $sel_id");
        $series_array = array();
        $initial_series_ndx = -1;
        $counter = 0;
        $variant = isset($plugin_cookies->variant) ? $plugin_cookies->variant : "auto";
        foreach ($list as $item) {
            if (!is_null($sel_id) && $item->id === $sel_id) {
                $initial_series_ndx = $counter;
                //hd_print("initial series idx: $initial_series_ndx");
            }

            $var = $variant;
            if (isset($item->variants) && array_key_exists($variant, $item->variants)) {
                $playback_url = $item->variants[$variant]->playback_url;
                $playback_url_is_stream_url = $item->variants[$variant]->playback_url_is_stream_url;
            } else {
                $var = 'default';
                $playback_url = $item->playback_url;
                $playback_url_is_stream_url = $item->playback_url_is_stream_url;
            }

            if ($initial_series_ndx === $counter)
                hd_print("starting playback url ($var): $playback_url");
            else
                hd_print("playback url ($var): $playback_url");

            $series_array[] = array(
                PluginVodSeriesInfo::name => $item->name,
                PluginVodSeriesInfo::playback_url => $playback_url,
                PluginVodSeriesInfo::playback_url_is_stream_url => $playback_url_is_stream_url,
            );
            $counter++;
        }

        return array(
            PluginVodInfo::id => $this->id,
            PluginVodInfo::name => $this->name,
            PluginVodInfo::description => $this->description,
            PluginVodInfo::poster_url => $this->poster_url,
            PluginVodInfo::series => $series_array,
            PluginVodInfo::initial_series_ndx => $initial_series_ndx,
            PluginVodInfo::buffering_ms => (isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000),
        );
    }
}

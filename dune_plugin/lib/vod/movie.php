<?php

require_once 'movie_series.php';

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
     * @var array
     */
    public $series_list;

    /**
     * @param string $id
     * @throws Exception
     */
    public function __construct($id)
    {
        if (is_null($id)) {
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

        $this->series_list = array();
    }

    /**
     * @param string $id
     * @param string $name
     * @param string $playback_url
     * @param bool $playback_url_is_stream_url
     * @throws Exception
     */
    public function add_series_data($id, $name, $playback_url, $playback_url_is_stream_url = true)
    {
        $this->series_list[] = new Movie_Series($id, $name, $playback_url, $playback_url_is_stream_url);
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

    /**
     * @param string $sel_id
     * @param int $buffering_ms
     * @return array
     * @throws Exception
     */
    public function get_vod_info($sel_id, $buffering_ms)
    {
        if (!is_array($this->series_list) ||
            count($this->series_list) === 0) {
            throw new Exception('Invalid movie: series list is empty');
        }

        $series_array = array();
        $initial_series_ndx = -1;
        foreach ($this->series_list as $ndx => $series) {
            if (!is_null($sel_id) && $series->id === $sel_id) {
                $initial_series_ndx = $ndx;
            }

            $series_array[] = array(
                PluginVodSeriesInfo::name => $series->name,
                PluginVodSeriesInfo::playback_url => $series->playback_url,
                PluginVodSeriesInfo::playback_url_is_stream_url => $series->playback_url_is_stream_url,
            );
        }

        return array(
            PluginVodInfo::id => $this->id,
            PluginVodInfo::name => $this->name,
            PluginVodInfo::description => $this->description,
            PluginVodInfo::poster_url => $this->poster_url,
            PluginVodInfo::series => $series_array,
            PluginVodInfo::initial_series_ndx => $initial_series_ndx,
            PluginVodInfo::buffering_ms => $buffering_ms,
        );
    }
}

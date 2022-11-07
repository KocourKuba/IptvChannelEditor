<?php

require_once 'movie_series.php';
require_once 'movie_season.php';
require_once 'movie_variant.php';

class Movie implements User_Input_Handler
{
    const ID = 'smart_movie';

    const HISTORY_LIST = 'history_items';

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
     * @var array|null
     */
    protected $playback_info;

    /**
     * @var string
     */
    protected $playback_series_ndx = -1;

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
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Movie: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case GUI_EVENT_PLAYBACK_STOP:
                if ($this->playback_info === null) break;

                //hd_print("movie: " . json_encode($this));
                if ($this->playback_series_ndx !== -1) {
                    $indexed_array = array_values($this->series_list);
                    $series = $indexed_array[$this->playback_series_ndx];
                } else {
                    $series = $this->series_list[$user_input->plugin_vod_id];
                }
                if (!empty($series->id)) {
                    $save_folder = HD::get_items('save_folder');
                    if (isset($save_folder[$user_input->plugin_vod_id][$series->id])) {
                        $save_folder[$user_input->plugin_vod_id][$series->id][] = $series->playback_url;
                        HD::put_items('save_folder', $save_folder);
                    }
                }

                if (!preg_match("/\.(?:mp3|wav|og(?:g|a)|flac|midi?|rm|m4a|aac|wma|mka|ape|jpg|png)$/i", $series->playback_url)) {
                    $viewed_items = HD::get_items('viewed_items');
                    $viewed_items[$series->playback_url] = $this->playback_info;
                    HD::put_items('viewed_items', $viewed_items);
                }

                if (isset($user_input->playback_stop_pressed)) {
                    $history_items = HD::get_items(self::HISTORY_LIST, true);
                    $history_items = array_reverse($history_items, true);
                    $history_items[$user_input->plugin_vod_id] = array(
                        'series' => $this->playback_series_ndx,
                        'info' => date("d.m.Y"),
                    );

                    $history_items = array_reverse($history_items, true);
                    if (count($history_items) === 64) {
                        array_pop($history_items);
                    }

                    HD::put_items(self::HISTORY_LIST, $history_items);
                    return Action_Factory::invalidate_folders(array(Starnet_Vod_Series_List_Screen::get_media_url_str($user_input->plugin_vod_id)));
                }
                break;

            case GUI_EVENT_TIMER:
                $actions = $this->get_movie_actions();
                $ext_command = file_get_contents('/tmp/run/ext_command.state');
                if (preg_match('/playback_position = (\d*)/', $ext_command, $m_pos)
                    && (int)$m_pos[1] > 0
                    && preg_match('/playback_duration = (\d*)/', $ext_command, $m_dur)
                    && (int)$m_dur[1] > 0) {
                    $this->playback_info = array($m_pos[1], $m_dur[1]);
                }

                $this->playback_series_ndx = $user_input->plugin_vod_series_ndx;
                return Action_Factory::change_behaviour($actions, 5000);
        }

        return null;
    }

    /**
     * @return array
     */
    protected function get_movie_actions()
    {
        User_Input_Handler_Registry::get_instance()->register_handler($this);
        $actions[GUI_EVENT_PLAYBACK_STOP] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLAYBACK_STOP);
        $actions[GUI_EVENT_TIMER] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_TIMER);
        return $actions;
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

        //hd_print("get_movie_play_info: selected screen: " . $media_url->get_raw_string());

        switch ($media_url->screen_id) {
            case Starnet_Vod_Seasons_List_Screen::ID:
                if (!is_array($this->season_list) || count($this->season_list) === 0) {
                    HD::print_backtrace();
                    throw new Exception("Invalid movie: season list is empty");
                }
                $list = $this->series_list;
                break;
            case Starnet_Vod_Series_List_Screen::ID:
                if (!is_array($this->series_list) || count($this->series_list) === 0) {
                    HD::print_backtrace();
                    throw new Exception("Invalid movie: series list is empty");
                }
                $list = $this->series_list;
                break;
            case Starnet_Vod_Movie_Screen::ID:
                $list = $this->series_list;
                break;
            default:
                HD::print_backtrace();
                throw new Exception("Unknown list screen: $media_url->screen_id");
        }

        $viewed_items = HD::get_items('viewed_items');
        $sel_id = isset($media_url->series_id) ? $media_url->series_id : null;
        //hd_print("selected id: $sel_id");
        $series_array = array();
        $initial_series_ndx = -1;
        $counter = 0;
        $variant = isset($plugin_cookies->variant) ? $plugin_cookies->variant : "auto";
        foreach ($list as $ndx => $series) {
            if (!is_null($sel_id) && $series->id === $sel_id) {
                $initial_series_ndx = $counter;
                //hd_print("initial series idx: $initial_series_ndx");
            }

            $var = $variant;
            if (isset($series->variants) && array_key_exists($variant, $series->variants)) {
                $playback_url = $series->variants[$variant]->playback_url;
                $playback_url_is_stream_url = $series->variants[$variant]->playback_url_is_stream_url;
            } else {
                $var = 'default';
                $playback_url = $series->playback_url;
                $playback_url_is_stream_url = $series->playback_url_is_stream_url;
            }

            if ($initial_series_ndx === $counter)
                hd_print("starting playback url ($var): $playback_url");
            else
                hd_print("playback url ($var): $playback_url");

            $suffix = '';
            if (isset($viewed_items[$playback_url])) {
                if ($viewed_items[$playback_url] === 'watched')
                    $suffix = ' [Просмотрено]';
                else{
                    $suffix = ' [Просмотр ' . format_duration($viewed_items[$playback_url][0]) . '/' . format_duration($viewed_items[$playback_url][1]) . ']';
                    $pos = (int)$viewed_items[$playback_url][0];
                    if ($pos < 0)
                        $pos = 0;
                    if ($pos + 5 >= (int)$viewed_items[$playback_url][1])
                        $pos = 0;
                    $initial_position_ms[$ndx] = $pos * 1000;
                }
            }

            if (!is_null($sel_id) && $playback_url === $sel_id)
                $initial_series_ndx = $ndx;

            $series_array[] = array(
                PluginVodSeriesInfo::name => $series->name . $suffix,
                PluginVodSeriesInfo::playback_url => $playback_url,
                PluginVodSeriesInfo::playback_url_is_stream_url => $playback_url_is_stream_url,
            );
            $counter++;
        }

        $ip_ms = 0;
        if (isset($initial_position_ms[$initial_series_ndx]))
            $ip_ms = $initial_position_ms[$initial_series_ndx];

        return array(
            PluginVodInfo::id => $this->id,
            PluginVodInfo::name => $this->name,
            PluginVodInfo::description => $this->description,
            PluginVodInfo::poster_url => $this->poster_url,
            PluginVodInfo::series => $series_array,
            PluginVodInfo::initial_series_ndx => $initial_series_ndx,
            PluginVodInfo::buffering_ms => (isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : 1000),
            PluginVodInfo::actions => $this->get_movie_actions(),
            PluginVodInfo::timer => Action_Factory::timer(5000),
            PluginVodInfo::initial_position_ms => $ip_ms,
        );
    }
}

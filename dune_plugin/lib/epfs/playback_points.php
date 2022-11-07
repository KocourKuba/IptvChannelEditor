<?php

###############################################################################
#
# Playback Points
#
# Author: Brigadir (forum.mydune.ru)
# Date: 30-01-2021
# Latest update: 23-01-2022
#
###############################################################################

require_once 'lib/mediaurl.php';
require_once 'lib/dune_stb_api.php';

///////////////////////////////////////////////////////////////////////////////

class Playback_Points
{
    /**
     * @var Playback_Points
     */
    private static $instance;

    /**
     * @var MediaUrl
     */
    private $curr_point;

    /**
     * @var MediaUrl[]
     */
    private $points;

    /**
     * @var string
     */
    private $tmp_path;

    ///////////////////////////////////////////////////////////////////////////

    private function __construct()
    {
        $this->tmp_path = get_temp_path('/watch_history.tmp');
    }

    /**
     * @return void
     */
    private function update_curr_point()
    {
        hd_print("Playback_Points::update_curr_point");

        if (!isset($this->curr_point->channel_id) || !class_exists("PluginRowsFolderView"))
            return;

        $time = time();
        $this->curr_point->time = $time;

        if (isset($this->curr_point->archive_tm) && ($this->curr_point->archive_tm > 0)) {
            $player_state = get_player_state_assoc();
            $time_position = max($time - $this->curr_point->time, 0);

            if (isset($player_state['playback_state']) && ($player_state['playback_state'] === PLAYBACK_PLAYING)) {
                $playback_position = empty($player_state['playback_position']) ? $this->curr_point->position + $time_position : $player_state['playback_position'];
                $this->curr_point->position = max($playback_position, 0);
            }
        }

        $this->points[$this->curr_point->channel_id] = $this->curr_point;
        file_put_contents($this->tmp_path, serialize($this->points));
    }

    /**
     * @param $channel_id
     * @param $archive_ts
     */
    private function push_point($channel_id, $archive_ts)
    {
        hd_print("Playback_Points::push_point channel_id $channel_id, archive_ts: $archive_ts");

        $prev_point = MediaURL::decode($this->curr_point->get_raw_string());
        if (!isset($prev_point->channel_id) || $prev_point->channel_id !== $channel_id) {
            $this->curr_point->channel_id = $channel_id;
            $this->curr_point->archive_tm = $archive_ts;
            $this->curr_point->position = 0;
            $this->curr_point->time = PHP_INT_MAX;

            $this->points = array($this->curr_point->channel_id => $this->curr_point) + $this->points;
        }
    }

    /**
     * @return MediaUrl|null
     */
    private function get_prev_point()
    {
        return isset($this->points[0]) ? $this->points[0] : null;
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @return void
     */
    public static function init()
    {
        if (is_null(self::$instance)) {
            hd_print("Playback_Points::init instance created");
            self::$instance = new Playback_Points();
            self::$instance->points = self::get_all();
        }

        self::$instance->curr_point = MediaURL::decode();
    }

    /**
     * @return void
     */
    public static function clear()
    {
        hd_print("Playback_Points::clear");
        if (file_exists(self::$instance->tmp_path)) {
            unlink(self::$instance->tmp_path);
        }
        self::$instance->points = array();
    }

    /**
     * @return void
     */
    public static function update()
    {
        if (is_null(self::$instance))
            self::init();

        self::$instance->update_curr_point();
    }

    /**
     * @param $channel_id
     * @param $archive_ts
     */
    public static function push($channel_id, $archive_ts)
    {
        if (is_null(self::$instance))
            self::init();

        $player_state = get_player_state_assoc();

        if (!isset($player_state['last_playback_event']) || ($player_state['last_playback_event'] !== PLAYBACK_PCR_DISCONTINUITY))
            self::$instance->push_point($channel_id, $archive_ts);
    }

    /**
     * @return MediaURL|null
     */
    public static function get_prev()
    {
        if (!is_null(self::$instance)
            && !is_null($point = self::$instance->get_prev_point())) {

            $media_url = MediaURL::decode();
            $media_url->channel_id = $point->channel_id;
            $media_url->archive_tm = $point->archive_tm;
            $media_url->position = $point->position;
            $media_url->time = $point->time;

            return $media_url;
        }

        return null;
    }

    /**
     * @return MediaURL[]|mixed
     */
    public static function get_all()
    {
        return file_exists(self::$instance->tmp_path) ? unserialize(file_get_contents(self::$instance->tmp_path)) : array();
    }
}

<?php

# Playback Points
#
# Idea: Brigadir (forum.mydune.ru)
# Modification: sharky72

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
     * @var string
     */
    private $curr_point_id;

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
        $this->tmp_path = get_data_path('/watch_history');
    }

    /**
     * @param string|null $id
     */
    private function update_point($id)
    {
        if (($this->curr_point_id === null && $id === null) || !class_exists("PluginRowsFolderView"))
            return;

        // update point for selected channel
        $id = ($id === null) ? $id : $this->curr_point_id;

        if (isset($this->points[$id]) && $this->points[$id]->archive_tm !== -1) {
            // if channel does support archive do not update current point
            $player_state = get_player_state_assoc();
            $time = time();

            if (isset($player_state['playback_state']) && ($player_state['playback_state'] === PLAYBACK_PLAYING)) {
                $playback_position = empty($player_state['playback_position']) ? max($time - $this->points[$id]->time, 0) : $player_state['playback_position'];
                hd_print("Playback_Points::update_point: playing position: $playback_position");
                $this->points[$id]->position = max($playback_position, 0);
            } else {
                $playback_position = max($time - $this->points[$id]->time, 0);
                $this->points[$id]->position = $playback_position;
            }
            hd_print("Playback_Points::update_point: playing position: $playback_position");
            $this->points[$id]->time = $time;

            file_put_contents($this->tmp_path, serialize($this->points));
        }
    }

    /**
     * @param string $channel_id
     * @param integer $archive_ts
     */
    private function push_point($channel_id, $archive_ts)
    {
        $player_state = get_player_state_assoc();

        if (isset($player_state['player_state']) && $player_state['player_state'] !== 'navigator') {
            if (!isset($player_state['last_playback_event']) || ($player_state['last_playback_event'] !== PLAYBACK_PCR_DISCONTINUITY)) {

                hd_print("Playback_Points::push_point channel_id $channel_id, archive_ts: $archive_ts");
                $this->curr_point_id = $channel_id;

                $point = MediaURL::decode();
                $point->channel_id = $channel_id;
                $point->archive_tm = $archive_ts;
                $point->position = 0;
                $point->time = PHP_INT_MAX;

                $this->points = array($channel_id => $point) + $this->points;
                if (count($this->points) > 7) {
                    array_pop($this->points);
                }
            }
        }
    }

    /**
     * @param string $id
     */
    private function erase_point($id)
    {
        hd_print("Playback_Points::erase " . ($id !== null ? $id : "all"));
        if ($id === null) {
            $this->points = array();
            if (file_exists($this->tmp_path)) {
                unlink($this->tmp_path);
            }
        } else {
            unset($this->points[$id]);
            file_put_contents($this->tmp_path, serialize($this->points));
        }
    }
    ///////////////////////////////////////////////////////////////////////////

    /**
     * @return void
     */
    public static function init()
    {
        if (is_null(self::$instance)) {
            self::$instance = new Playback_Points();
        }

        if (!isset(self::$instance->points)) {
            self::$instance->points = self::get_all();
        }
    }

    /**
     * @return void
     */
    public static function clear($id = null)
    {
        if (is_null(self::$instance))
            self::init();

        self::$instance->erase_point($id);
    }

    /**
     * @return void
     */
    public static function update($id = null)
    {
        if (is_null(self::$instance))
            self::init();

        self::$instance->update_point($id);
    }

    /**
     * @param $channel_id
     * @param $archive_ts
     */
    public static function push($channel_id, $archive_ts)
    {
        if (is_null(self::$instance))
            self::init();

        self::$instance->push_point($channel_id, $archive_ts);
    }

    /**
     * @return MediaURL[]|mixed
     */
    public static function get_all()
    {
        $points = file_exists(self::$instance->tmp_path) ? unserialize(file_get_contents(self::$instance->tmp_path)) : array();
        hd_print("Loaded playback_points: " . count($points));
        return $points;
    }
}

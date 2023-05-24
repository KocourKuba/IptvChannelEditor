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
    const TV_HISTORY_ITEMS = 'tv_history_items';

    /**
     * @var Playback_Points
     */
    private static $instance;

    /**
     * @var string
     */
    private $curr_point_id;

    /**
     * @var array
     */
    private $points;

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param string|null $id
     */
    private function update_point($id)
    {
        //hd_print("Playback_Points::update_point");

        if ($this->curr_point_id === null && $id === null)
            return;

        // update point for selected channel
        $id = ($id === null) ? $id : $this->curr_point_id;

        if (isset($this->points[$id])) {
            $player_state = get_player_state_assoc();
            if (isset($player_state['playback_state'], $player_state['playback_position'])
                && ($player_state['playback_state'] === PLAYBACK_PLAYING || $player_state['playback_state'] === PLAYBACK_STOPPED)) {

                // if channel does support archive do not update current point
                $this->points[$id] += ($this->points[$id] !== 0) ? $player_state['playback_position'] : 0;
                //hd_print("Playback_Points::update_point channel_id $id at time mark: {$this->points[$id]}");
            }
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

                hd_print("Playback_Points::push_point channel_id $channel_id time mark: $archive_ts");
                $this->curr_point_id = $channel_id;

                if (isset($this->points[$channel_id])) {
                    unset($this->points[$channel_id]);
                }
                $this->points = array($channel_id => $archive_ts) + $this->points;
                if (count($this->points) > 7) {
                    array_pop($this->points);
                }
            }
        }
    }

    /**
     */
    private function save_points()
    {
        hd_print("Playback_Points::save_points: " . count($this->points));
        HD::put_items(self::TV_HISTORY_ITEMS, $this->points);
    }

    /**
     * @param string $id
     */
    private function erase_point($id)
    {
        hd_print("Playback_Points::erase " . ($id !== null ? $id : "all"));
        if ($id === null) {
            $this->points = array();
            HD::erase_items(self::TV_HISTORY_ITEMS);
        } else {
            unset($this->points[$id]);
            HD::put_items(self::TV_HISTORY_ITEMS, $this->points);
        }
    }
    ///////////////////////////////////////////////////////////////////////////

    /**
     * @return void
     */
    public static function init()
    {
        if (is_null(self::$instance)) {
            self::$instance = new self();
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
     */
    public static function save()
    {
        if (is_null(self::$instance))
            self::init();

        self::$instance->save_points();
    }

    /**
     * @return MediaURL[]|mixed
     */
    public static function get_all()
    {
        $points = HD::get_items(self::TV_HISTORY_ITEMS, true);
        hd_print("Loaded playback_points: " . count($points));
        while (count($points) > 7) {
            array_pop($points);
        }

        return $points;
    }
}

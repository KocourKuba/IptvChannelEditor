<?php
/**
 * The MIT License (MIT)
 *
 * @Author: sharky72 (https://github.com/KocourKuba)
 * @Idea: Brigadir (forum.mydune.ru)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

require_once 'mediaurl.php';
require_once 'dune_stb_api.php';

///////////////////////////////////////////////////////////////////////////////

class Playback_Points
{
    /**
     * @var array
     */
    private $playback_points;

    /**
     * @var Default_Dune_Plugin
     */
    private $plugin;

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct($plugin)
    {
        $this->plugin = $plugin;
        $this->load_points();
    }

    /**
     * @return array
     */
    public function get_all()
    {
        return $this->playback_points;
    }

    /**
     * @return int
     */
    public function size()
    {
        return count($this->playback_points);
    }

    /**
     * @param bool $force
     * @return void
     */
    public function load_points($force = false)
    {
        if (!isset($this->playback_points) || $force) {
            $path = $this->get_playback_points_file();
            if (empty($path)) {
                $this->playback_points = array();
                return;
            }

            $this->playback_points = HD::get_items($path);
            hd_debug_print(count($this->playback_points) . " from: $path", true);
            while (count($this->playback_points) > 7) {
                array_pop($this->playback_points);
            }
        }
    }

    /**
     * @return void
     */
    public function save()
    {
        $path = $this->get_playback_points_file();
        if (empty($path)) {
            return;
        }

        if (count($this->playback_points) !== 0) {
            hd_debug_print(count($this->playback_points) . " to: $path", true);
            HD::put_items($path, $this->playback_points);
        } else if (file_exists($path)) {
            unlink($path);
        }
    }

    /**
     * /**
     * * Called when first start url
     * *
     * @param string $channel_id
     * @param int $channel_ts
     */
    public function push_point($channel_id, $channel_ts)
    {
        hd_debug_print(null, true);
        $player_state = get_player_state_assoc();
        $state = safe_get_value($player_state, PLAYER_STATE);
        $event = safe_get_value($player_state, LAST_PLAYBACK_EVENT);
        if ($state !== null && $state !== PLAYER_STATE_NAVIGATOR && $event !== PLAYBACK_PCR_DISCONTINUITY) {
            $ts_str = format_datetime("Y-m-d H:i", $channel_ts);
            hd_debug_print("Push playback point for channel: '$channel_id' at time mark: $channel_ts ($ts_str)", true);
            $this->playback_points[$channel_id][PARAM_CHANNEL_TS] = $channel_ts;
        }
    }

    /**
     * Called when playing stop
     *
     * @param string $id
     */
    public function update_point($id)
    {
        hd_debug_print(null, true);
        if (empty($id) || !isset($this->playback_points[$id])) {
            return;
        }

        // update point for selected channel
        $player_state = get_player_state_assoc();
        $state = safe_get_value($player_state, PLAYBACK_STATE);
        $position = safe_get_value($player_state, PLAYBACK_POSITION, 0);
        if ($state !== PLAYBACK_PLAYING && $state !== PLAYBACK_STOPPED) {
            return;
        }

        if ($this->playback_points[$id][PARAM_CHANNEL_TS] !== 0) {
            $this->playback_points[$id][PARAM_CHANNEL_TS] += $position;
            $channel_ts = $this->playback_points[$id][PARAM_CHANNEL_TS];
        } else {
            $channel_ts = 0;
        }

        if ($channel_ts !== 0) {
            $prog_info = $this->plugin->get_epg_info($id, $channel_ts);
            if (isset($prog_info[PluginTvEpgProgram::start_tm_sec])) {
                $this->playback_points[$id][PARAM_START_TM] = $prog_info[PluginTvEpgProgram::start_tm_sec];
                $this->playback_points[$id][PARAM_END_TM] = $prog_info[PluginTvEpgProgram::end_tm_sec];
            }

            $this->save();
            $ts_str = format_datetime("Y-m-d H:i", $channel_ts);
            hd_debug_print("Save playback point for channel '$id' at time mark: $channel_ts ($ts_str)", true);
        }
    }

    /**
     * @param string $id
     */
    public function erase_point($id)
    {
        hd_debug_print("erase $id");
        unset($this->playback_points[$id]);
        $this->save();
    }

    /**
     * @return void
     */
    public function clear_points()
    {
        hd_debug_print();
        $this->playback_points = array();
        $this->save();
    }
    ///////////////////////////////////////////////////////////////////////////

    /**
     * @return string
     */
    private function get_playback_points_file()
    {
        $path = $this->plugin->get_history_path();
        if (!create_path($path)) {
            hd_debug_print("History path not exist: $path");
            return '';
        }

        $channel_list = $this->plugin->get_setting(PARAM_CHANNELS_LIST_NAME, 'default');
        $channel_list = empty($channel_list) ? 'default' : $channel_list;
        return $this->plugin->get_history_path(Hashed_Array::hash($channel_list));
    }
}

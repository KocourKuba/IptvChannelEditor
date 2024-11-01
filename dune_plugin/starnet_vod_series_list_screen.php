<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Series_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_series';
    const ACTION_QUALITY_SELECTED = 'select_quality';
    const ACTION_AUDIO_SELECTED = 'select_audio';

    /**
     * @var array
     */
    protected $qualities;

    /**
     * @var array
     */
    protected $audios;

    /**
     * @var array
     */
    protected $default_audio;

    /**
     * @param $movie_id
     * @param string|null $season_id
     * @param string|null $episode_id
     * @return false|string
     */
    public static function get_media_url_string($movie_id, $season_id = null, $episode_id = null)
    {
        $arr = array('screen_id' => self::ID, 'movie_id' => $movie_id);
        if ($season_id !== null) {
            $arr['season_id'] = $season_id;
        }

        if ($episode_id !== null) {
            $arr['episode_id'] = $episode_id;
        }

        return MediaURL::encode($arr);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $action_play = User_Input_Handler_Registry::create_action($this, ACTION_PLAY_ITEM);

        $actions = array(
            GUI_EVENT_KEY_ENTER   => $action_play,
            GUI_EVENT_KEY_PLAY    => $action_play,
            GUI_EVENT_KEY_B_GREEN => User_Input_Handler_Registry::create_action($this, ACTION_WATCHED, TR::t('vod_screen_viewed_not_viewed')),
        );

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_QUALITY_SUPPORTED)) {
            $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id);
            $current_quality = $this->plugin->get_parameter(PARAM_VOD_DEFAULT_QUALITY, 'auto');
            if (!is_null($movie) && isset($movie->qualities_list) && count($movie->qualities_list) > 1) {
                $q_exist = (in_array($current_quality, $movie->qualities_list) ? "" : "? ");
                $actions[GUI_EVENT_KEY_D_BLUE] = User_Input_Handler_Registry::create_action($this,
                    ACTION_QUALITY,
                    TR::t('vod_screen_quality__1', "$q_exist$current_quality"));
            }
        }

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_AUDIO_SUPPORTED)) {
            $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id);
            if (!is_null($movie)) {
                $selected_audio = isset($this->default_audio[$media_url->movie_id]) ? $this->default_audio[$media_url->movie_id] : 'auto';
                $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this,
                    ACTION_AUDIO,
                    TR::t('vod_screen_audio__1', $selected_audio));
            }
        }

        $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU);

        return $actions;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $selected_media_url = MediaURL::decode($user_input->selected_media_url);

        switch ($user_input->control_id) {
            case ACTION_PLAY_ITEM:
                try {
                    $vod_info = $this->plugin->vod->get_vod_info($selected_media_url);
                    $post_action = $this->plugin->vod_player_exec(
                        $vod_info,
                        isset($user_input->external),
                        isset($user_input->external)
                            ? User_Input_Handler_Registry::create_action($this, ACTION_WATCHED, null, array(ACTION_WATCHED => true))
                            : null
                    );
                } catch (Exception $ex) {
                    hd_debug_print("Movie can't played");
                    print_backtrace_exception($ex);
                    return Action_Factory::show_title_dialog(
                        TR::t('err_channel_cant_start'),
                        null,
                        TR::t('warn_msg2__1', $ex->getMessage())
                    );
                }

                return $post_action;

            case ACTION_QUALITY:
                $movie = $this->plugin->vod->get_loaded_movie($selected_media_url->movie_id);
                if (is_null($movie)) break;

                $menu_items = array();
                if (!isset($this->qualities) || count($this->qualities) < 2) break;

                $current_quality = $this->plugin->get_parameter(PARAM_VOD_DEFAULT_QUALITY, 'auto');

                $menu_items[] = $this->plugin->create_menu_item($this,
                    self::ACTION_QUALITY_SELECTED,
                    TR::t('by_default'),
                    $current_quality === 'auto' ? 'check.png' : null,
                    array('quality' => 'auto')
                );

                foreach ($this->qualities as $key => $quality) {
                    if ($key === 'auto') continue;

                    $icon = null;
                    if ((string)$key === $current_quality) {
                        $icon = 'gui_skin://small_icons/video_settings.aai';
                    }
                    $menu_items[] = $this->plugin->create_menu_item($this, self::ACTION_QUALITY_SELECTED, $quality->name, $icon, array('quality' => $key));
                }

                return Action_Factory::show_popup_menu($menu_items);

            case self::ACTION_QUALITY_SELECTED:
                if (!isset($this->qualities)) {
                    break;
                }

                $quality = 'auto';
                foreach ($this->qualities as $key => $value) {
                    if ($user_input->quality === (string)$key) {
                        $quality = $user_input->quality;
                    }
                }

                $this->plugin->set_parameter(PARAM_VOD_DEFAULT_QUALITY, $quality);
                $parent_url = MediaURL::decode($user_input->parent_media_url);
                return Action_Factory::change_behaviour($this->get_action_map($parent_url, $plugin_cookies));

            case ACTION_AUDIO:
                $movie = $this->plugin->vod->get_loaded_movie($selected_media_url->movie_id);
                if (is_null($movie)) break;

                $menu_items = array();
                if (!isset($this->audios[$selected_media_url->episode_id]) || count($this->audios[$selected_media_url->episode_id]) < 2) break;

                $audios['auto'] = TR::t('by_default');
                $audios = array_merge($audios, $this->audios[$selected_media_url->episode_id]);
                $selected_audio = isset($this->default_audio[$selected_media_url->movie_id]) ? $this->default_audio[$selected_media_url->movie_id] : 'auto';
                foreach ($audios as $key => $audio) {
                    $name = ($key === 'auto') ? $audio : $audio->name;
                    $icon = null;
                    if ((string)$key === $selected_audio) {
                        $icon = 'gui_skin://small_icons/audiot_settings.aai';
                    }
                    $menu_items[] = $this->plugin->create_menu_item($this, self::ACTION_AUDIO_SELECTED, $name, $icon, array('audio' => $key));
                }

                return Action_Factory::show_popup_menu($menu_items);

            case self::ACTION_AUDIO_SELECTED:
                if (!isset($this->audios[$selected_media_url->episode_id])) {
                    break;
                }

                $audio = 'auto';
                foreach ($this->audios[$selected_media_url->episode_id] as $key => $value) {
                    if ($user_input->audio === (string)$key) {
                        $audio = $user_input->audio;
                        break;
                    }
                }

                $this->default_audio[$selected_media_url->movie_id] = $audio;

                $parent_url = MediaURL::decode($user_input->parent_media_url);
                return Action_Factory::change_behaviour($this->get_action_map($parent_url, $plugin_cookies));

            case ACTION_WATCHED:
                $movie = $this->plugin->vod->get_loaded_movie($selected_media_url->movie_id);
                if (is_null($movie)) break;

                $viewed_items = $this->plugin->vod->get_history_movies();
                if ($viewed_items !== null) {
                    if ((isset($user_input->{ACTION_WATCHED}) && $user_input->{ACTION_WATCHED} !== false)
                        || !isset($viewed_items[$selected_media_url->movie_id][$selected_media_url->episode_id])) {
                        $movie_info[Movie::WATCHED_FLAG] = true;
                        $movie_info[Movie::WATCHED_DATE] = time();
                    } else {
                        $movie_info = $viewed_items[$selected_media_url->movie_id][$selected_media_url->episode_id];
                        if ($movie_info[Movie::WATCHED_FLAG] !== false) {
                            $movie_info = null;
                        } else {
                            $movie_info[Movie::WATCHED_FLAG] = true;
                            $movie_info[Movie::WATCHED_DATE] = time();
                        }
                    }

                    if ($movie_info === null) {
                        unset($viewed_items[$selected_media_url->movie_id][$selected_media_url->episode_id]);
                        if (empty($viewed_items[$selected_media_url->movie_id])) {
                            unset($viewed_items[$selected_media_url->movie_id]);
                        }
                    } else {
                        $viewed_items[$selected_media_url->movie_id][$selected_media_url->episode_id] = $movie_info;
                    }
                    $this->plugin->vod->set_history_movies($viewed_items);
                }

                $range = $this->get_folder_range(MediaURL::decode($user_input->parent_media_url), 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true, $user_input->sel_ndx);

            case GUI_EVENT_KEY_POPUP_MENU:
                if (!is_limited_apk()) {
                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_PLAY_ITEM,
                        TR::t('tv_screen_external_player'),
                        'play.png',
                        array('external' => true));

                    return Action_Factory::show_popup_menu($menu_items);
                }

                return null;

            default:
                hd_debug_print("default");
        }

        return null;
    }
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        hd_debug_print($media_url, true);

        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id);
        if (is_null($movie)) {
            return array();
        }

        $viewed_items = $this->plugin->vod->get_history_movies();
        $viewed_item = isset($viewed_items[$media_url->movie_id]) ? $viewed_items[$media_url->movie_id] : array();

        $items = array();
        foreach ($movie->series_list as $episode) {
            if (isset($media_url->season_id) && $media_url->season_id !== $episode->season_id) continue;

            $info = $episode->name;
            $color = 15;
            if (isset($viewed_item[$episode->id])) {
                $item_info = $viewed_item[$episode->id];
                hd_debug_print("viewed item: " . json_encode($item_info));
                if ($item_info[Movie::WATCHED_FLAG]) {
                    $date = format_datetime("d.m.Y H:i", $item_info[Movie::WATCHED_DATE]);
                    $info = TR::t('vod_screen_viewed__2', $episode->name, $date);
                } else if (isset($item_info[Movie::WATCHED_DURATION]) && $item_info[Movie::WATCHED_DURATION] !== -1) {
                    $start = format_duration_seconds($item_info[Movie::WATCHED_POSITION]);
                    $total = format_duration_seconds($item_info[Movie::WATCHED_DURATION]);
                    $date = format_datetime("d.m.Y H:i", $item_info[Movie::WATCHED_DATE]);
                    $info = $episode->name . " [$start/$total] $date";
                }

                $color = 5;
            }

            hd_debug_print("Movie media url: " . self::get_media_url_string($movie->id, $episode->season_id, $episode->id), true);
            if (!empty($episode->qualities)) {
                $this->qualities = $episode->qualities;
                hd_debug_print("Qualities: " . json_encode($episode->qualities), true);
            }

            if (!empty($episode->audios)) {
                $this->audios[$episode->id] = $episode->audios;
                hd_debug_print("Audio: " . pretty_json_format($episode->audios), true);
            }

            $items[] = array(
                PluginRegularFolderItem::media_url => self::get_media_url_string($movie->id, $episode->season_id, $episode->id),
                PluginRegularFolderItem::caption => $info,
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai',
                    ViewItemParams::item_detailed_info => empty($episode->series_desc) ? $episode->name : $episode->series_desc,
                    ViewItemParams::item_detailed_icon_path => empty($episode->movie_image) ? 'gui_skin://large_icons/movie.aai' : $episode->movie_image,
                    ViewItemParams::item_caption_color => $color,
                ),
            );
        }

        return $items;
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('list_1x11_small_info'),
            $this->plugin->get_screen_view('list_1x11_info'),
        );
    }
}

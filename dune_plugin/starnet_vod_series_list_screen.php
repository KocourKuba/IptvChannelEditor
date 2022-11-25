<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Series_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_series';

    const ACTION_WATCHED = 'watched';
    const ACTION_QUALITY = 'quality';
    const ACTION_POPUP_MENU = 'popup_menu';
    const ACTION_EXTERNAL_PLAYER = 'use_external_player';

    /**
     * @var array
     */
    protected $variants;

    /**
     * @param $movie_id
     * @param null $series_id
     * @return false|string
     */
    public static function get_media_url_str($movie_id, $series_id = null)
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'movie_id' => $movie_id, 'series_id' => $series_id));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->GET_VOD_SERIES_FOLDER_VIEW());

        if ($plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        // hd_print("get_action_map: " . $media_url->get_raw_string());
        $actions = array();
        $actions[GUI_EVENT_KEY_ENTER] = Action_Factory::vod_play();
        $actions[GUI_EVENT_KEY_PLAY] = Action_Factory::vod_play();

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_WATCHED);
        $add_action['caption'] = 'Просмотрено/Не просмотрено';
        $actions[GUI_EVENT_KEY_B_GREEN] = $add_action;

        if ($this->plugin->config->get_feature(Plugin_Constants::VOD_QUALITY_SUPPORTED)) {
            $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
            $variant = isset($plugin_cookies->variant) ? $plugin_cookies->variant : "auto";
            if (!is_null($movie) && isset($movie->variants_list)) {

                $q_exist = (in_array($variant, $movie->variants_list) ? "" : "?");
                $quality_action = User_Input_Handler_Registry::create_action($this, self::ACTION_QUALITY);
                $quality_action['caption'] = "Качество - $variant$q_exist";
                $actions[GUI_EVENT_KEY_D_BLUE] = $quality_action;
            }
        }

        if (is_android()) {
            $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, self::ACTION_POPUP_MENU);
        }

        return $actions;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Vod Vod_Series_List_Screen: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case self::ACTION_QUALITY:
                $menu_items = array();
                if (!isset($this->variants) || count($this->variants) < 2) break;

                foreach ($this->variants as $key => $variant) {
                    $add_action = User_Input_Handler_Registry::create_action($this, $key);
                    $menu_items[] = array(GuiMenuItemDef::caption => $key, GuiMenuItemDef::action => $add_action);
                }
                return Action_Factory::show_popup_menu($menu_items);

            case self::ACTION_WATCHED:
                if (!isset($user_input->selected_media_url))
                    return null;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
                if (is_null($movie)) {
                    return null;
                }

                $this->plugin->vod->ensure_history_loaded($plugin_cookies);
                $viewed_items = $this->plugin->vod->get_history_movies();

                if ((isset($user_input->{self::ACTION_WATCHED}) && $user_input->{self::ACTION_WATCHED} !== false)
                    || !isset($viewed_items[$media_url->movie_id][$user_input->sel_ndx])) {
                    $movie_info[Movie::WATCHED_FLAG] = true;
                } else {
                    $movie_info = $viewed_items[$media_url->movie_id][$user_input->sel_ndx];
                    if ($movie_info[Movie::WATCHED_FLAG] !== false) {
                        $movie_info[Movie::WATCHED_FLAG] = false;
                        $movie_info[Movie::WATCHED_POSITION] = 0;
                        $movie_info[Movie::WATCHED_DURATION] = -1;
                    } else {
                        $movie_info[Movie::WATCHED_FLAG] = true;
                    }
                }

                $viewed_items[$media_url->movie_id][$user_input->sel_ndx] = $movie_info;
                $this->plugin->vod->set_history_items($viewed_items, $plugin_cookies);

                if (isset($user_input->parent_media_url)) {
                    //hd_print("Refresh folder");
                    $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                    $sel_ndx = $user_input->sel_ndx;
                    if ($sel_ndx < 0)
                        $sel_ndx = 0;
                    $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                } else {
                    //hd_print("not set");
                    break;
                }

                //hd_print("range: " . json_encode($range));
                return Action_Factory::update_regular_folder($range, true, $sel_ndx);

            case self::ACTION_EXTERNAL_PLAYER:
                try {
                    $add_params[self::ACTION_WATCHED] = true;
                    $post_action = User_Input_Handler_Registry::create_action($this, self::ACTION_WATCHED, $add_params);
                    $info = $this->plugin->get_vod_info($user_input->selected_media_url, $plugin_cookies);
                    if (isset($info[PluginVodInfo::initial_series_ndx], $info[PluginVodInfo::series][$info[PluginVodInfo::initial_series_ndx]])) {
                        $series = $info[PluginVodInfo::series];
                        $idx = $info[PluginVodInfo::initial_series_ndx];
                        $url = $series[$idx][PluginVodSeriesInfo::playback_url];
                        $param_pos = strpos($url, '|||dune_params');
                        $url =  $param_pos!== false ? substr($url, 0, $param_pos) : $url;
                        $cmd = 'am start -d "' . $url . '" -t "video/*" -a android.intent.action.VIEW 2>&1';
                        hd_print("play movie in the external player: $cmd");
                        exec($cmd);
                        return $post_action;
                    }
                } catch (Exception $e) {
                    hd_print("Movie can't played, exception info: " . $e->getMessage());
                }
                break;

            case self::ACTION_POPUP_MENU:
                $add_ext_player_action = User_Input_Handler_Registry::create_action($this, self::ACTION_EXTERNAL_PLAYER);
                $caption = 'Проиграть внешним плейером';
                $menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_ext_player_action);
                return Action_Factory::show_popup_menu($menu_items);

            default:
                if (isset($this->variants)) {
                    foreach ($this->variants as $key => $variant) {
                        if ($user_input->control_id !== (string)$key) continue;

                        $plugin_cookies->variant = $key;
                        $parent_url = MediaURL::decode($user_input->parent_media_url);
                        return Action_Factory::change_behaviour($this->get_action_map($parent_url, $plugin_cookies));
                    }
                }
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
        //hd_print("Vod_Series_List_Screen::get_all_folder_items: MediaUrl: " . $media_url->get_raw_string());

        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();

        $this->plugin->vod->ensure_history_loaded($plugin_cookies);
        $viewed_items = $this->plugin->vod->get_history_movies();

        $viewed_item = isset($viewed_items[$media_url->movie_id]) ? $viewed_items[$media_url->movie_id] : array();
        $counter = 0;
        //hd_print("movie_id: $movie->id");
        foreach ($movie->series_list as $series) {
            if (isset($media_url->season_id) && $media_url->season_id !== $series->season_id) continue;

            //hd_print("series_idx: $counter name: $series->name pb_url: $series->playback_url");
            if (isset($viewed_item[$counter])) {
                $item_info = $viewed_item[$counter];

                if ($item_info[Movie::WATCHED_FLAG]) {
                    $info = "$series->name | [Просмотрено]";
                } else if (isset($item_info[Movie::WATCHED_DURATION])) {
                    if ($item_info[Movie::WATCHED_DURATION] === -1) {
                        $info = $series->name;
                    } else {
                        $start = format_duration_seconds($item_info[Movie::WATCHED_POSITION]);
                        $total = format_duration_seconds($item_info[Movie::WATCHED_DURATION]);
                        $date = format_datetime("d.m.Y H:i", $item_info[Movie::WATCHED_DATE]);
                        $info = $series->name . " [$start/$total] $date";
                    }
                } else {
                    $info = $series->name;
                }

                $color = 5;
            } else {
                $color = 15;
                $info = $series->name;
            }

            $this->variants = $series->variants;
            $items[] = array
            (
                PluginRegularFolderItem::media_url => self::get_media_url_str($movie->id, $series->id),
                PluginRegularFolderItem::caption => $info,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai',
                    ViewItemParams::item_detailed_info => $series->series_desc,
                    ViewItemParams::item_caption_color => $color,
                ),
            );

            $counter++;
        }

        return $items;
    }
}

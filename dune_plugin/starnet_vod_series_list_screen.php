<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Series_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_series';

    const ACTION_WATCHED = 'watched';
    const ACTION_QUALITY = 'quality';
    const ACTION_REFRESH = 'refresh';

    const VIEWED_LIST = 'viewed_items';

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

                $q_exist = (in_array($variant, $movie->variants_list) ? "" : "*");
                $quality_action = User_Input_Handler_Registry::create_action($this, self::ACTION_QUALITY);
                $quality_action['caption'] = "Качество - $variant$q_exist";
                $actions[GUI_EVENT_KEY_D_BLUE] = $quality_action;
            }
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
                $viewed_items = HD::get_items(self::VIEWED_LIST);

                $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
                if (is_null($movie)) {
                    return null;
                }

                $playback_url = $movie->series_list[$media_url->series_id]->playback_url;
                $is_viewed = array_key_exists($playback_url, $viewed_items);
                if ($is_viewed) {
                    unset ($viewed_items[$playback_url]);
                } else {
                    $viewed_items[$playback_url] = 'watched';
                }
                HD::put_items(self::VIEWED_LIST, $viewed_items);

                $perform_new_action = User_Input_Handler_Registry::create_action($this, self::ACTION_REFRESH);
                return Action_Factory::invalidate_folders(array(self::ID), $perform_new_action);

            case self::ACTION_REFRESH:
                if (!isset($user_input->parent_media_url))
                    return null;

                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $sel_ndx = $user_input->sel_ndx + 1;
                if ($sel_ndx < 0)
                    $sel_ndx = 0;
                $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true, $sel_ndx);

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
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("Vod_Series_List_Screen::get_all_folder_items: MediaUrl: " . $media_url->get_raw_string());
        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();

        $viewed_items = HD::get_items(self::VIEWED_LIST);
        foreach ($movie->series_list as $series) {
            if (isset($media_url->season_id) && $media_url->season_id !== $series->season_id) continue;

            //hd_print("movie_id: $movie->id name: $series->name series_id: $series->id pb_url: $series->playback_url");
            if (isset($viewed_items[$series->playback_url])) {
                $viewed_item = $viewed_items[$series->playback_url];
                if ($viewed_item === 'watched')
                    $prefix = '[Просмотрено]';
                else
                    $prefix = '[' . format_duration_seconds($viewed_item[0]) . '/' . format_duration_seconds($viewed_item[1]) . ']';

                $info = "$series->name | $prefix";
                $color = 3;
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
        }

        return $items;
    }
}

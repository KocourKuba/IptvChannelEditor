<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Vod_Series_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_series';

    /**
     * @var array
     */
    protected $variants;

    /**
     * @param $movie_id
     * @return false|string
     */
    public static function get_media_url_str($movie_id)
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'movie_id' => $movie_id));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_VOD_SERIES_FOLDER_VIEW());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
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

        if ($this->plugin->config->get_feature(VOD_PORTAL_SUPPORTED)) {
            $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
            $variant = isset($plugin_cookies->variant) ? $plugin_cookies->variant : "auto";
            if (!is_null($movie) && isset($movie->variants_list)) {

                $q_exist = (in_array($variant, $movie->variants_list) ? "": "*");
                $quality_action = User_Input_Handler_Registry::create_action($this, 'show_quality');
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
        //foreach ($user_input as $key => $value)
        //    hd_print("  $key => $value");

        if ($user_input->control_id ===  'show_quality') {
            $menu_items = array();
            if (isset($this->variants) && count($this->variants) > 1) {
                foreach ($this->variants as $key => $variant) {
                    $add_action = User_Input_Handler_Registry::create_action($this, $key);
                    $caption = $key;
                    $menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_action);
                }
                return Action_Factory::show_popup_menu($menu_items);
            }
        } else if (isset($this->variants)) {
            foreach ($this->variants as $key => $variant) {
                if ($user_input->control_id !== (string)$key) continue;

                $plugin_cookies->variant = $key;
                $parent_url = MediaURL::decode($user_input->parent_media_url);
                return Action_Factory::change_behaviour($this->get_action_map($parent_url, $plugin_cookies));
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

        foreach ($movie->series_list as $series) {
            if (isset($media_url->season_id) && $media_url->season_id !== $series->season_id) continue;

            //hd_print("movie_id: $movie->id name: $series->name series_id: $series->id");
            $this->variants = $series->variants;
            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array
                (
                    'screen_id' => self::ID,
                    'movie_id' => $movie->id,
                    'series_id' => $series->id,
                )),
                PluginRegularFolderItem::caption => $series->name,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai',
                    ViewItemParams::item_detailed_info => $series->series_desc,
                ),
            );
        }

        return $items;
    }

    /**
     * @param MediaURL $media_url
     * @return null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}

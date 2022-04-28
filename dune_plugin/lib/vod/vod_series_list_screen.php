<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Vod_Series_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_series';

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
     * @param $user_input
     * @param $plugin_cookies
     * @return null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('Vod Vod_Series_List_Screen: handle_user_input:');
        foreach ($user_input as $key => $value)
            hd_print("  $key => $value");

        if ($user_input->control_id === 'add_actions'){
            if (!isset($user_input->selected_media_url))
                return null;

            $media_url = MediaURL::decode($user_input->selected_media_url);
            if (isset($media_url->is_movie) && $media_url->is_movie === false)
                return Action_Factory::open_folder();

            return Action_Factory::vod_play();
        }
        return null;
    }
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array
        (
            GUI_EVENT_KEY_ENTER => User_Input_Handler_Registry::create_action($this, 'add_actions'),
            GUI_EVENT_KEY_PLAY => User_Input_Handler_Registry::create_action($this, 'add_actions'),
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_print("Vod_Series_List_Screen::get_all_folder_items: MediaUrl: " . $media_url->get_raw_string());
        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();

        foreach ($movie->series_list as $series) {
            if (isset($media_url->season_id) && $media_url->season_id !== $series->season_id) continue;

            //hd_print("movie_id: $movie->id name: $series->name series_id: $series->id");
            $is_movie = empty($series->variants);
            $icon = $is_movie ? 'gui_skin://small_icons/movie.aai' : 'gui_skin://small_icons/folder.aai';
            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array
                (
                    'screen_id' => $is_movie ? self::ID : Vod_Variants_List_Screen::ID,
                    'movie_id' => $is_movie ? $movie->id : $series->id,
                    'series_id' => $is_movie ? $series->id : null,
                    'is_movie' => $is_movie,
                )),
                PluginRegularFolderItem::caption => $series->name,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $icon,
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

<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Seasons_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_seasons';

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
     * @param $user_input
     * @param $plugin_cookies
     * @return null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
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
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_PLAY => Action_Factory::open_folder(),
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
        hd_print("Vod_Seasons_List_Screen::get_all_folder_items: MediaUrl: " . $media_url->get_raw_string());
        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();

        foreach ($movie->season_list as $season) {
            hd_print("movie_id: $movie->id season_id: $season->id season_name: $season->name");
            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array
                (
                    'screen_id' => Starnet_Vod_Series_List_Screen::ID,
                    'movie_id' => $movie->id,
                    'season_id' => $season->id,
                )),
                PluginRegularFolderItem::caption => $season->name,
                PluginRegularFolderItem::view_item_params => array(ViewItemParams::icon_path => 'gui_skin://small_icons/folder.aai'),
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

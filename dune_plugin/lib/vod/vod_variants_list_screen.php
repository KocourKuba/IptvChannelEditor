<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Vod_Variants_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_variants';

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
            GUI_EVENT_KEY_ENTER => Action_Factory::vod_play(),
            GUI_EVENT_KEY_PLAY => Action_Factory::vod_play(),
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
        //hd_print("Vod_Variants_List_Screen::get_all_folder_items: MediaUrl: " . $media_url->get_raw_string());
        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();
        foreach ($movie->series_list[$movie->id]->variants as $variant) {
            //hd_print("variant: $variant->name movie_id: $movie->id series_id: $variant->id");
            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array
                (
                    'screen_id' => self::ID,
                    'movie_id' => $movie->id,
                    'series_id' => $variant->id,
                )),
                PluginRegularFolderItem::caption => $variant->name,
                PluginRegularFolderItem::view_item_params => array(ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai'),
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

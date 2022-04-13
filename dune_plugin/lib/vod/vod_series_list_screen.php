<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class VodSeriesListScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'vod_series';

    public static function get_media_url_str($movie_id)
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'movie_id' => $movie_id));
    }

    ///////////////////////////////////////////////////////////////////////

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_VOD_SERIES_FOLDER_VIEW());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        return null;
    }
    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array
        (
            GUI_EVENT_KEY_ENTER => ActionFactory::vod_play(),
            GUI_EVENT_KEY_PLAY => ActionFactory::vod_play(),
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);

        $movie = $this->plugin->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if (is_null($movie)) {
            return array();
        }

        $items = array();

        foreach ($movie->series_list as $series) {
            // hd_print("name: $series->name movie_id: $movie->id series_id: $series->id");
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
                ),
            );
        }

        return $items;
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}

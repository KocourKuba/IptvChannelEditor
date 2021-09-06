<?php
require_once 'vod.php';
require_once 'lib/abstract_preloaded_regular_screen.php';

class VodSeriesListScreen extends AbstractPreloadedRegularScreen
{
    const ID = 'vod_series';

    public static function get_media_url_str($movie_id)
    {
        return MediaURL::encode(
            array('screen_id' => self::ID, 'movie_id' => $movie_id));
    }

    ///////////////////////////////////////////////////////////////////////

    private $vod;

    public function __construct(Vod $vod)
    {
        $this->vod = $vod;

        parent::__construct(self::ID, $this->get_folder_views());
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

    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->vod->folder_entered($media_url, $plugin_cookies);

        $movie = $this->vod->get_loaded_movie($media_url->movie_id, $plugin_cookies);
        if ($movie === null) {
            return array();
        }

        $items = array();

        foreach ($movie->series_list as $series) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url =>
                    MediaURL::encode(
                        array
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

    ///////////////////////////////////////////////////////////////////////

    private function get_folder_views()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => true,
                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 1,
                    ViewParams::num_rows => 12,
                    ViewParams::paint_path_box =>true,
					ViewParams::paint_details => true,
					ViewParams::paint_item_info_in_details => true,
					ViewParams::item_detailed_info_auto_line_break => true,
					ViewParams::item_detailed_info_title_color => 10,
					ViewParams::item_detailed_info_text_color => 15,
                    ViewParams::background_path=> 'plugin_file://icons/bg.jpg',
					ViewParams::background_order => 0,
					ViewParams::background_height => 1080,
					ViewParams::background_width => 1920,
					ViewParams::optimize_full_screen_background => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::item_caption_dx => 60,
                    ViewItemParams::icon_path => 'gui_skin://small_icons/movie.aai'
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->vod->get_archive($media_url);
    }
}

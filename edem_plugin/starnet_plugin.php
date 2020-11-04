<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/default_dune_plugin.php';
require_once 'lib/utils.php';

require_once 'lib/tv/tv_group_list_screen.php';
require_once 'lib/tv/tv_channel_list_screen.php';
require_once 'lib/tv/tv_favorites_screen.php';

require_once 'lib/vod/vod_list_screen.php';
require_once 'lib/vod/vod_movie_screen.php';
require_once 'lib/vod/vod_series_list_screen.php';
require_once 'lib/vod/vod_favorites_screen.php';

require_once 'starnet_config.php';

require_once 'starnet_tv.php';
require_once 'starnet_vod.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';
require_once 'starnet_vod_list_screen.php';
require_once 'starnet_main_screen.php';

///////////////////////////////////////////////////////////////////////////

class DemoPlugin extends DefaultDunePlugin
{
    public function __construct()
    {
        $this->tv = new DemoTv();
        $this->vod = new DemoVod();

        $tv_folder_views = $this->get_tv_folder_views();

        //$this->add_screen(new TvGroupListScreen($this->tv, DemoConfig::GET_TV_GROUP_LIST_FOLDER_VIEWS()));
        $this->add_screen(new TvChannelListScreen($this->tv, DemoConfig::GET_TV_CHANNEL_LIST_FOLDER_VIEWS()));
        $this->add_screen(new TvFavoritesScreen($this->tv, DemoConfig::GET_TV_CHANNEL_LIST_FOLDER_VIEWS()));
        $this->add_screen(new Starnet_MainScreen($this->tv, DemoConfig::GET_TV_GROUP_LIST_FOLDER_VIEWS()));
        $this->add_screen(new DemoSetupScreen());
        $this->add_screen(new VodFavoritesScreen($this->vod));
        $this->add_screen(new DemoVodCategoryListScreen());
        $this->add_screen(new DemoVodListScreen($this->vod));
        $this->add_screen(new VodMovieScreen($this->vod));
        $this->add_screen(new VodSeriesListScreen($this->vod));
    }

    ///////////////////////////////////////////////////////////////////////

    private function get_tv_folder_views()
    {
        return DemoConfig::GET_TV_CHANNEL_LIST_FOLDER_VIEWS();
    }
}

///////////////////////////////////////////////////////////////////////////
?>

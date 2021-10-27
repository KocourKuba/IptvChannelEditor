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

require_once 'plugin_type.php';
require_once 'starnet_tv.php';
require_once 'starnet_vod.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_search_screen.php';
require_once 'starnet_vod_category_list_screen.php';
require_once 'starnet_vod_list_screen.php';
require_once 'starnet_main_screen.php';
require_once 'starnet_entry_handler.php';
require_once 'smart_file_screen.php';

class StarnetDunePlugin extends DefaultDunePlugin
{
    private $entry_handler;
    /**
     * @throws Exception
     */
    public function __construct()
    {
        parent::__construct();

        $plugin_type = PLUGIN_TYPE;
        if(!class_exists($plugin_type) || !is_subclass_of($plugin_type, 'DefaultConfig'))
            throw new Exception('Unknown plugin type: ' . $plugin_type);

        $config = new $plugin_type;
        StarnetPluginTv::$config = $config;
        StarnetMainScreen::$config = $config;
        StarnetSetupScreen::$config = $config;

        hd_print("Plugin name:     " . $config::$PLUGIN_SHOW_NAME);
        hd_print("Plugin version:  " . $config::$PLUGIN_VERSION);
        hd_print("Plugin date:     " . $config::$PLUGIN_DATE);
        hd_print("Plugin config:   " . $plugin_type);
        hd_print("Account type:    " . $config::$ACCOUNT_TYPE);
        hd_print("TV fav:          " . ($config::$TV_FAVORITES_SUPPORTED ? "yes" : "no"));
        hd_print("VOD page:        " . ($config::$VOD_MOVIE_PAGE_SUPPORTED ? "yes" : "no"));
        hd_print("VOD fav:         " . ($config::$VOD_FAVORITES_SUPPORTED ? "yes" : "no"));
        hd_print("MPEG-TS support: " . ($config::$MPEG_TS_SUPPORTED ? "yes" : "no"));

        $tv = new StarnetPluginTv();
        $this->tv = $tv;
        $this->entry_handler = new StarnetEntryHandler();
        $this->add_screen(new StarnetMainScreen($tv, $config->GET_TV_GROUP_LIST_FOLDER_VIEWS()));
        $this->add_screen(new TvChannelListScreen($tv, $config->GET_TV_CHANNEL_LIST_FOLDER_VIEWS()));
        $this->add_screen(new StarnetSetupScreen($tv));
        $this->add_screen(new SmartFileScreen());

        if ($config::$TV_FAVORITES_SUPPORTED) {
            $this->add_screen(new TvFavoritesScreen($tv, $config->GET_TV_CHANNEL_LIST_FOLDER_VIEWS()));
        }

        if ($config::$VOD_MOVIE_PAGE_SUPPORTED) {
            StarnetVod::$config = $config;
            StarnetVodListScreen::$config = $config;
            StarnetVodCategoryListScreen::$config = $config;
            VodMovieScreen::$config = $config;

            $vod = new StarnetVod();
            $this->vod = $vod;

            $this->add_screen(new StarnetSearchScreen($vod));
            $this->add_screen(new VodFavoritesScreen($vod));
            $this->add_screen(new StarnetVodCategoryListScreen());
            $this->add_screen(new StarnetVodListScreen($vod));
            $this->add_screen(new VodMovieScreen($vod));
            $this->add_screen(new VodSeriesListScreen($vod));
        }
    }
}

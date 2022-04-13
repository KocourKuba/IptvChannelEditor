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
require_once 'starnet_filter_screen.php';
require_once 'starnet_vod_category_list_screen.php';
require_once 'starnet_vod_list_screen.php';
require_once 'starnet_main_screen.php';
require_once 'starnet_entry_handler.php';
require_once 'starnet_folder_screen.php';

class StarnetDunePlugin extends DefaultDunePlugin
{
    /**
     * @throws Exception
     */
    public function __construct()
    {
        parent::__construct();

        $this->plugin_path = __DIR__;
        $plugin_type = PLUGIN_TYPE;
        if (!class_exists($plugin_type) || !is_subclass_of($plugin_type, 'DefaultConfig')) {
            hd_print("Unknown plugin type: $plugin_type");
            throw new Exception("Unknown plugin type: $plugin_type");
        }

        $config = new $plugin_type;
        $this->config = $config;

        print_sysinfo();

        hd_print("----------------------------------------------------");
        hd_print("Plugin name:      " . $config->PLUGIN_SHOW_NAME);
        hd_print("Plugin version:   " . $config->PLUGIN_VERSION);
        hd_print("Plugin date:      " . $config->PLUGIN_DATE);
        hd_print("Account type:     " . $config->get_feature(ACCOUNT_TYPE));
        hd_print("TV fav:           " . ($config->get_feature(TV_FAVORITES_SUPPORTED) ? "yes" : "no"));
        hd_print("VOD page:         " . ($config->get_feature(VOD_MOVIE_PAGE_SUPPORTED) ? "yes" : "no"));
        hd_print("VOD fav:          " . ($config->get_feature(VOD_FAVORITES_SUPPORTED) ? "yes" : "no"));
        hd_print("----------------------------------------------------");

        UserInputHandlerRegistry::get_instance()->register_handler(new StarnetEntryHandler());

        $this->tv = new StarnetPluginTv($this);
        $this->vod = new StarnetVod($this);

        $this->main_screen = new StarnetMainScreen($this);
        $this->tv_channels_screen = new TvChannelListScreen($this);
        $this->setup_screen = new StarnetSetupScreen($this);
        $this->folder_screen = new StarnetFolderScreen($this);
        $this->favorites_screen = new TvFavoritesScreen($this);
        $this->search_screen = new StarnetSearchScreen($this);
        $this->vod_favorites_screen = new VodFavoritesScreen($this);
        $this->vod_category_list_Screen = new StarnetVodCategoryListScreen($this);
        $this->vod_list_screen = new StarnetVodListScreen($this);
        $this->vod_movie_screen = new VodMovieScreen($this);
        $this->vod_series_list_screen = new VodSeriesListScreen($this);
        $this->filter_screen = new StarnetFilterScreen($this);

        hd_print("Init done.");
    }
}

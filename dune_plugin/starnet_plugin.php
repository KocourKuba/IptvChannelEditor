<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/default_dune_plugin.php';
require_once 'lib/hd.php';

require_once 'lib/tv/tv_group_list_screen.php';
require_once 'lib/tv/tv_channel_list_screen.php';
require_once 'lib/tv/tv_favorites_screen.php';

require_once 'lib/vod/vod_list_screen.php';
require_once 'lib/vod/vod_movie_screen.php';
require_once 'lib/vod/vod_seasons_list_screen.php';
require_once 'lib/vod/vod_series_list_screen.php';
require_once 'lib/vod/vod_favorites_screen.php';
require_once 'lib/vod/vod_history_screen.php';

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

class Starnet_Plugin extends Default_Dune_Plugin
{
    /**
     * @throws Exception
     */
    public function __construct()
    {
        parent::__construct();

        $this->plugin_path = __DIR__;
        $plugin_type = PLUGIN_TYPE;
        if (!class_exists($plugin_type) || !is_subclass_of($plugin_type, 'Default_Config')) {
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
        hd_print("LocalTime         " . format_datetime('Y-m-d H:i', time()));
        hd_print("TimeZone          " . getTimeZone());
        hd_print("Daylight          " . date('I'));
        hd_print("Icon              " . $config->PLUGIN_ICON);
        hd_print("Background        " . $config->PLUGIN_BACKGROUND);
        hd_print("----------------------------------------------------");

        User_Input_Handler_Registry::get_instance()->register_handler(new Starnet_Entry_Handler());

        $this->tv = new Starnet_Tv($this);
        $this->vod = new Starnet_Vod($this);

        $this->main_screen = new Starnet_Main_Screen($this);
        $this->tv_channels_screen = new Tv_Channel_List_Screen($this);
        $this->setup_screen = new Starnet_Setup_Screen($this);
        $this->folder_screen = new Starnet_Folder_Screen($this);
        $this->favorites_screen = new Tv_Favorites_Screen($this);
        $this->search_screen = new Starnet_Search_Screen($this);
        $this->vod_favorites_screen = new Vod_Favorites_Screen($this);
        $this->vod_category_list_Screen = new Starnet_Vod_Category_List_Screen($this);
        $this->vod_list_screen = new Starnet_Vod_List_Screen($this);
        $this->vod_movie_screen = new Vod_Movie_Screen($this);
        $this->vod_season_List_Screen = new Vod_Seasons_List_Screen($this);
        $this->vod_series_list_screen = new Vod_Series_List_Screen($this);
        $this->vod_history_screen = new Vod_History_Screen($this);
        $this->filter_screen = new Starnet_Filter_Screen($this);

        hd_print("Init done.");
    }
}

<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/hd.php';
require_once 'lib/default_dune_plugin.php';

require_once 'starnet_entry_handler.php';
require_once 'starnet_tv_groups_screen.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_channels_setup_screen.php';
require_once 'starnet_interface_setup_screen.php';
require_once 'starnet_interface_newui_setup_screen.php';
require_once 'starnet_epg_setup_screen.php';
require_once 'starnet_streaming_setup_screen.php';
require_once 'starnet_ext_setup_screen.php';
require_once 'starnet_folder_screen.php';
require_once 'starnet_tv.php';
require_once 'starnet_tv_channel_list_screen.php';
require_once 'starnet_tv_favorites_screen.php';
require_once 'starnet_tv_history_screen.php';
require_once 'starnet_vod.php';
require_once 'starnet_vod_search_screen.php';
require_once 'starnet_vod_filter_screen.php';
require_once 'starnet_vod_category_list_screen.php';
require_once 'starnet_vod_list_screen.php';
require_once 'starnet_vod_movie_screen.php';
require_once 'starnet_vod_seasons_list_screen.php';
require_once 'starnet_vod_series_list_screen.php';
require_once 'starnet_vod_favorites_screen.php';
require_once 'starnet_vod_history_screen.php';
require_once 'starnet_epfs_handler.php';

class Starnet_Plugin extends Default_Dune_Plugin
{
    /**
     * @throws Exception
     */
    public function __construct()
    {
        parent::__construct();

        $this->plugin_setup();

        User_Input_Handler_Registry::get_instance()->register_handler(new Starnet_Entry_Handler($this));

        $this->tv = new Starnet_Tv($this);
        $this->vod = new Starnet_Vod($this);

        $this->create_screen(new Starnet_Tv_Groups_Screen($this));
        $this->create_screen(new Starnet_Tv_Channel_List_Screen($this));
        $this->create_screen(new Starnet_Tv_Favorites_Screen($this));
        $this->create_screen(new Starnet_TV_History_Screen($this));

        $this->create_screen(new Starnet_Setup_Screen($this));
        $this->create_screen(new Starnet_Channels_Setup_Screen($this));
        $this->create_screen(new Starnet_Interface_Setup_Screen($this));
        if (HD::rows_api_support()) {
            $this->create_screen(new Starnet_Interface_NewUI_Setup_Screen($this));
        }
        $this->create_screen(new Starnet_Epg_Setup_Screen($this));
        $this->create_screen(new Starnet_Streaming_Setup_Screen($this));
        $this->create_screen(new Starnet_Ext_Setup_Screen($this));

        $this->create_screen(new Starnet_Folder_Screen($this));

        if ($this->config->get_feature(Plugin_Constants::VOD_ENGINE) !== "None") {
            $this->create_screen(new Starnet_Vod_Favorites_Screen($this));
            $this->create_screen(new Starnet_Vod_History_Screen($this));
            $this->create_screen(new Starnet_Vod_Category_List_Screen($this));
            $this->create_screen(new Starnet_Vod_List_Screen($this));
            $this->create_screen(new Starnet_Vod_Movie_Screen($this));
            $this->create_screen(new Starnet_Vod_Seasons_List_Screen($this));
            $this->create_screen(new Starnet_Vod_Series_List_Screen($this));
            $this->create_screen(new Starnet_Vod_Search_Screen($this));
        }

        if ($this->config->get_feature(Plugin_Constants::VOD_FILTER_SUPPORTED)) {
            $this->create_screen(new Starnet_Vod_Filter_Screen($this));
        }

        Starnet_Epfs_Handler::init($this);

        hd_debug_print("Plugin loading complete.");
    }
}

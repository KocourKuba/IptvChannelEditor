<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/default_dune_plugin.php';
require_once 'lib/utils.php';

require_once 'lib/tv/tv_group_list_screen.php';
require_once 'lib/tv/tv_channel_list_screen.php';
require_once 'lib/tv/tv_favorites_screen.php';
require_once 'lib/views_config.php';

require_once 'lib/vod/vod_list_screen.php';
require_once 'lib/vod/vod_movie_screen.php';
require_once 'lib/vod/vod_series_list_screen.php';
require_once 'lib/vod/vod_favorites_screen.php';

require_once 'plugin_type.php';
require_once 'starnet_tv.php';
require_once 'starnet_vod.php';
require_once 'starnet_setup_screen.php';
require_once 'starnet_vod_category_list_screen.php';
require_once 'starnet_vod_list_screen.php';
require_once 'starnet_main_screen.php';

///////////////////////////////////////////////////////////////////////////

class StarnetDunePlugin extends DefaultDunePlugin
{
    /**
     * @throws Exception
     */
    public function __construct()
    {
        parent::__construct();

        $plugin_type = PLUGIN_TYPE;
        if(!class_exists($plugin_type) || !is_subclass_of($plugin_type, 'IConfig'))
            throw new Exception('Unknown plugin type: ' . $plugin_type);

        $this->tv = new StarnetPluginTv(new $plugin_type);
        $bg_picture = $this->tv->config->get_bg_picture();
        $this->add_screen(new TvChannelListScreen($this->tv, ViewsConfig::GET_TV_CHANNEL_LIST_FOLDER_VIEWS($bg_picture)));
        $this->add_screen(new TvFavoritesScreen($this->tv, ViewsConfig::GET_TV_CHANNEL_LIST_FOLDER_VIEWS($bg_picture)));
        $this->add_screen(new StarnetMainScreen($this->tv, ViewsConfig::GET_TV_GROUP_LIST_FOLDER_VIEWS($bg_picture)));
        $this->add_screen(new StarnetSetupScreen($this->tv));
    }
}

///////////////////////////////////////////////////////////////////////////
?>

<?php
///////////////////////////////////////////////////////////////////////////

require_once 'media_url.php';
require_once 'tv/epg_iterator.php';
require_once 'tv/tv.php';
require_once 'user_input_handler_registry.php';
require_once 'action_factory.php';
require_once 'control_factory.php';
require_once 'control_factory_ext.php';

class DefaultDunePlugin implements DunePlugin
{
    /**
     * @var string
     */
    public $plugin_path;
    /**
     * @var StarnetPluginTv
     */
    public $tv;
    /**
     * @var StarnetVod
     */
    public $vod;
    /**
     * @var DefaultConfig
     */
    public $config;

    /**
     * @var StarnetMainScreen
     */
    public $main_screen;
    /**
     * @var TvChannelListScreen
     */
    public $tv_channels_screen;
    /**
     * @var StarnetSetupScreen
     */
    public $setup_screen;
    /**
     * @var StarnetFolderScreen
     */
    public $folder_screen;
    /**
     * @var TvFavoritesScreen
     */
    public $favorites_screen;
    /**
     * @var StarnetSearchScreen
     */
    public $search_screen;
    /**
     * @var VodFavoritesScreen
     */
    public $vod_favorites_screen;
    /**
     * @var StarnetVodCategoryListScreen
     */
    public $vod_category_list_Screen;
    /**
     * @var StarnetVodListScreen
     */
    public $vod_list_screen;
    /**
     * @var VodMovieScreen
     */
    public $vod_movie_screen;
    /**
     * @var VodSeriesListScreen
     */
    public $vod_series_list_screen;
    /**
     * @var StarnetFilterScreen
     */
    public $filter_screen;

    /**
     * @var array
     */
    private $screens;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct()
    {
        $this->screens = array();
    }

    public function create_screen(&$object)
    {
        if (!is_null($object) && method_exists($object, 'get_id')) {
            hd_print('create_screen: ' . get_class($object));
            $this->add_screen($object);
            UserInputHandlerRegistry::get_instance()->register_handler($object);
        } else {
            hd_print(get_class($object) . ': Screen class is illegal. get_id method not defined!');
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Screen support.
    //
    ///////////////////////////////////////////////////////////////////////

    protected function add_screen($scr)
    {
        if (isset($this->screens[$scr->get_id()])) {
            hd_print("Error: screen (id: " . $scr->get_id() . ") already registered.");
        } else {
            $this->screens[$scr->get_id()] = $scr;
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    protected function get_screen_by_id($screen_id)
    {
        if (isset($this->screens[$screen_id])) {
            // hd_print("get_screen_by_id: '$screen_id'");
            return $this->screens[$screen_id];
        }

        hd_print("Error: no screen with id '$screen_id' found.");
        throw new Exception('Screen not found');
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    protected function get_screen_by_url($media_url)
    {
        $screen_id = isset($media_url->screen_id) ? $media_url->screen_id : $media_url->get_raw_string();

        return $this->get_screen_by_id($screen_id);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Folder support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_folder_view($media_url, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_next_folder_view($media_url, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_next_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_regular_folder_items($media_url, $from_ndx, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_folder_range($decoded_media_url, $from_ndx, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // IPTV channels support (TV support).
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_tv_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_info: TV is not supported');
            throw new Exception('TV is not supported');
        }

        $decoded_media_url = MediaURL::decode($media_url);

        return $this->tv->get_tv_info($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_tv_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_stream_url: TV is not supported');
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_playback_url: TV is not supported');
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_day_epg($channel_id, $day_start_tm_sec, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_day_epg: TV is not supported');
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_day_epg($channel_id, $day_start_tm_sec, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function change_tv_favorites($op_type, $channel_id, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('change_tv_favorites: TV is not supported');
            throw new Exception('TV is not supported');
        }

        return $this->tv->change_tv_favorites($op_type, $channel_id, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // VOD support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_vod_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print('get_vod_info: VOD is not supported');
            throw new Exception('VOD is not supported');
        }

        $decoded_media_url = MediaURL::decode($media_url);

        return $this->vod->get_vod_info($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_vod_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print('get_vod_stream_url: VOD is not supported');
            throw new Exception('VOD is not supported');
        }

        return $this->vod->get_vod_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Misc.
    //
    ///////////////////////////////////////////////////////////////////////

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        return UserInputHandlerRegistry::get_instance()->handle_user_input(
            $user_input, $plugin_cookies);
    }

    public function get_image_path($image = null)
    {
        return "$this->plugin_path/img/" . ($image === null ?: $image);
    }
}

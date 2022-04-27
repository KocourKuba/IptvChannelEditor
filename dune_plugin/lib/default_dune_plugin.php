<?php
///////////////////////////////////////////////////////////////////////////

require_once 'mediaurl.php';
require_once 'tv/epg_iterator.php';
require_once 'tv/tv.php';
require_once 'user_input_handler_registry.php';
require_once 'action_factory.php';
require_once 'control_factory.php';
require_once 'control_factory_ext.php';

class Default_Dune_Plugin implements DunePlugin
{
    /**
     * @var string
     */
    public $plugin_path;
    /**
     * @var Starnet_Tv
     */
    public $tv;
    /**
     * @var Starnet_Vod
     */
    public $vod;
    /**
     * @var Default_Config
     */
    public $config;

    /**
     * @var Starnet_Main_Screen
     */
    public $main_screen;
    /**
     * @var Tv_Channel_List_Screen
     */
    public $tv_channels_screen;
    /**
     * @var Starnet_Setup_Screen
     */
    public $setup_screen;
    /**
     * @var Starnet_Folder_Screen
     */
    public $folder_screen;
    /**
     * @var Tv_Favorites_Screen
     */
    public $favorites_screen;
    /**
     * @var Starnet_Search_Screen
     */
    public $search_screen;
    /**
     * @var Vod_Favorites_Screen
     */
    public $vod_favorites_screen;
    /**
     * @var Starnet_Vod_Category_List_Screen
     */
    public $vod_category_list_Screen;
    /**
     * @var Starnet_Vod_List_Screen
     */
    public $vod_list_screen;
    /**
     * @var Vod_Movie_Screen
     */
    public $vod_movie_screen;
    /**
     * @var Vod_Seasons_List_Screen
     */
    public $vod_season_List_Screen;
    /**
     * @var Vod_Series_List_Screen
     */
    public $vod_series_list_screen;
    /**
     * @var Vod_Variants_List_Screen
     */
    public $vod_variants_list_screen;
    /**
     * @var Starnet_Filter_Screen
     */
    public $filter_screen;

    /**
     * @var array|Screen[]
     */
    private $screens;

    ///////////////////////////////////////////////////////////////////////

    protected function __construct()
    {
        $this->screens = array();
    }

    /**
     * @param $object
     */
    public function create_screen(&$object)
    {
        if (!is_null($object) && method_exists($object, 'get_id')) {
            hd_print('create_screen: ' . get_class($object));
            $this->add_screen($object);
            User_Input_Handler_Registry::get_instance()->register_handler($object);
        } else {
            hd_print(get_class($object) . ': Screen class is illegal. get_id method not defined!');
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Screen support.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Screen $scr
     */
    protected function add_screen(Screen $scr)
    {
        if (isset($this->screens[$scr->get_id()])) {
            hd_print("Error: screen (id: " . $scr->get_id() . ") already registered.");
        } else {
            $this->screens[$scr->get_id()] = $scr;
        }
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $screen_id
     * @return Screen
     * @throws Exception
     */
    protected function get_screen_by_id($screen_id)
    {
        if (isset($this->screens[$screen_id])) {
            // hd_print("get_screen_by_id: '$screen_id'");
            return $this->screens[$screen_id];
        }

        hd_print("Error: no screen with id '$screen_id' found.");
        HD::print_backtrace();
        throw new Exception('Screen not found');
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @return Screen
     * @throws Exception
     */
    protected function get_screen_by_url(MediaURL $media_url)
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
     * @param string $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_folder_view($media_url, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_next_folder_view($media_url, &$plugin_cookies)
    {
        $decoded_media_url = MediaURL::decode($media_url);

        return $this->get_screen_by_url($decoded_media_url)->get_next_folder_view($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param int $from_ndx
     * @param $plugin_cookies
     * @return array
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
     * @param string $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_tv_info($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_info: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        $decoded_media_url = MediaURL::decode($media_url);

        return $this->tv->get_tv_info($decoded_media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return string
     * @throws Exception
     */
    public function get_tv_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_stream_url: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param int $archive_tm_sec
     * @param string $protect_code
     * @param $plugin_cookies
     * @return string
     * @throws Exception
     */
    public function get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_tv_playback_url: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_tv_playback_url($channel_id, $archive_tm_sec, $protect_code, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $channel_id
     * @param int $day_start_tm_sec
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_day_epg($channel_id, $day_start_tm_sec, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('get_day_epg: TV is not supported');
            HD::print_backtrace();
            throw new Exception('TV is not supported');
        }

        return $this->tv->get_day_epg($channel_id, $day_start_tm_sec, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $op_type
     * @param string $channel_id
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function change_tv_favorites($op_type, $channel_id, &$plugin_cookies)
    {
        if (is_null($this->tv)) {
            hd_print('change_tv_favorites: TV is not supported');
            HD::print_backtrace();
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
     * @param string $media_url_str
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_vod_info($media_url_str, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print('get_vod_info: VOD is not supported');
            HD::print_backtrace();
            throw new Exception('VOD is not supported');
        }

        hd_print("Default_Dune_Plugin::get_vod_info: MediaUrl: $media_url_str");
        $media_url = MediaURL::decode($media_url_str);

        return $this->vod->get_vod_info($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $media_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_stream_url($media_url, &$plugin_cookies)
    {
        if (is_null($this->vod)) {
            hd_print('get_vod_stream_url: VOD is not supported');
            return '';
        }

        return $this->vod->get_vod_stream_url($media_url, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Misc.
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        return User_Input_Handler_Registry::get_instance()->handle_user_input($user_input, $plugin_cookies);
    }

    /**
     * @param string $image
     * @return string
     */
    public function get_image_path($image = null)
    {
        return "$this->plugin_path/img/" . ($image === null ?: $image);
    }
}

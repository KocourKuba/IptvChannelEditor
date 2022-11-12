<?php
////////////////////////////////////////////////////////////////////////////////

require_once "starnet_tv_rows_screen.php";

require_once 'lib/dune_stb_api.php';
require_once "lib/epfs/config.php";
require_once "lib/epfs/abstract_epfs_handler.php";
require_once "lib/epfs/dummy_epfs_screen.php";

////////////////////////////////////////////////////////////////////////////////

class Starnet_Epfs_Handler extends Abstract_Epfs_Handler
{
    /**
     * @var string
     */
    protected static $epf_id;

    /**
     * @var string
     */
    protected static $no_internet_epfs;

    /**
     * @var Starnet_Tv_Rows_Screen
     */
    protected static $tv_rows_screen;

    /**
     * @var Dummy_Epfs_Screen
     */
    protected static $dummy_epf_screen;

    protected static $no_internet_epfs_created = false;

    ////////////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     * @throws Exception
     */
    public static function init(Default_Dune_Plugin $plugin)
    {
        self::$epf_id = $plugin->plugin_info['app_name'];
        self::$no_internet_epfs = self::$epf_id;

        hd_print("Starnet_Epfs_Handler: init: epf_id: " . self::$epf_id);
        parent::initialize(self::$epf_id);

        if (!class_exists("PluginRowsFolderView"))
            return;

        self::$tv_rows_screen = new Starnet_Tv_Rows_Screen($plugin);
        $plugin->create_screen(self::$tv_rows_screen);

        self::$dummy_epf_screen = new Dummy_Epfs_Screen($plugin);
        $plugin->create_screen(self::$dummy_epf_screen);
    }

    /**
     * @param $first_run
     * @param $plugin_cookies
     */
    private static function ensure_no_internet_epfs_created($first_run, &$plugin_cookies)
    {
        if (self::$no_internet_epfs_created)
            return;

        $path = self::get_no_internet_epf_path(self::$no_internet_epfs);

        if ($first_run || !is_file($path)) {
            self::write_no_internet_epf(self::$no_internet_epfs, self::$dummy_epf_screen->get_folder_view_for_epf(true, $plugin_cookies));
        }

        self::$no_internet_epfs_created = true;
    }

    /**
     * @return mixed
     */
    public static function get_epf()
    {
        return json_decode(self::read_epf_data(self::$epf_id));
    }

    /**
     * @return void
     */
    public static function need_update_epf_mapping()
    {
        if (!empty(self::$tv_rows_screen))
            self::$tv_rows_screen->need_update_epf_mapping_flag = true;
    }

    /**
     * @param array|null $media_urls
     * @param $post_action
     * @return array
     */
    public static function invalidate_folders($media_urls = null, $post_action = null)
    {
        $arr = array_merge(array(self::$epf_id), (is_array($media_urls) ? $media_urls : array()));
        return Action_Factory::invalidate_folders($arr, $post_action);
    }

    /**
     * @param $plugin_cookies
     */
    public static function update_tv_epfs(&$plugin_cookies)
    {
        self::update_all_epfs(false, $plugin_cookies);
    }

    /**
     * @param $first_run
     * @param $plugin_cookies
     * @return array|null
     */
    public static function update_all_epfs($first_run, &$plugin_cookies)
    {
        hd_print("update_all_epfs: first run " . ($first_run ? "yes" : "no"));

        self::ensure_no_internet_epfs_created($first_run, $plugin_cookies);

        try {
            $folder_view = self::$tv_rows_screen->get_folder_view_for_epf($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Exception while generating epf: " . $e->getMessage());
            return null;
        }

        $cold_run = !is_file(self::warmed_up_path());
        hd_print("Cold run: " . ($cold_run ? "yes" : "no"));
        if ($cold_run) {
            file_put_contents(self::warmed_up_path(), '');
        }

        $epf_data = json_encode($folder_view);
        if ($epf_data !== self::read_epf_data(self::$epf_id)) {
            self::write_epf_data(self::$epf_id, $epf_data);
        }

        return Action_Factory::status(0);
    }
}

<?php
require_once 'starnet_vod_category.php';
require_once 'starnet_vod_list_screen.php';

class StarnetVodCategoryListScreen extends AbstractPreloadedRegularScreen
{
    const ID = 'vod_category_list';
    public static $config = null;
    private $category_list;
    private $category_index;

    public function __construct()
    {
        parent::__construct(self::ID, $this->get_folder_views());
    }

    public static function get_media_url_str($category_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'category_id' => $category_id,
            ));
    }

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
        );
    }

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $config = self::$config;

        if (is_null($this->category_index) || is_null($this->category_list))
            $config->fetch_vod_categories($plugin_cookies, $this->category_list, $this->category_index);

        $category_list = $this->category_list;

        if (isset($media_url->category_id)) {
            if (!isset($this->category_index[$media_url->category_id])) {
                hd_print("Error: parent category (id: " . $media_url->category_id . ") not found.");
                throw new Exception('No parent category found');
            }

            $parent_category = $this->category_index[$media_url->category_id];
            $category_list = $parent_category->get_sub_categories();
        }

        $items = array();

        if ($config::$VOD_FAVORITES_SUPPORTED && !isset($media_url->category_id)) {

            $items[] = array
            (
                PluginRegularFolderItem::media_url => VodFavoritesScreen::get_media_url_str(),
                PluginRegularFolderItem::caption => DefaultConfig::FAV_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => DefaultConfig::FAV_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => 4,
                    ViewItemParams::item_detailed_icon_path => DefaultConfig::FAV_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        // Search
        $items[] = array
        (
            PluginRegularFolderItem::media_url => 'search_screen',
            PluginRegularFolderItem::caption => DefaultConfig::SEARCH_MOVIES_CATEGORY_CAPTION,
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => DefaultConfig::SEARCH_MOVIES_CATEGORY_ICON_PATH,
                ViewItemParams::item_caption_color => 14,
                ViewItemParams::item_detailed_icon_path => DefaultConfig::SEARCH_MOVIES_CATEGORY_ICON_PATH,
            )
        );

        foreach ($category_list as $category) {
            $media_url_str = is_null($category->get_sub_categories()) ?
                StarnetVodListScreen::get_media_url_str($category->get_id(), null) :
                self::get_media_url_str($category->get_id());

            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $category->get_caption(),
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $category->get_icon_path(),
                    ViewItemParams::item_detailed_icon_path => $category->get_icon_path()
                )
            );
        }

        return $items;
    }

    ///////////////////////////////////////////////////////////////////////

    private function get_folder_views()
    {
        return self::$config->GET_VOD_CATEGORY_LIST_FOLDER_VIEWS();
    }
}

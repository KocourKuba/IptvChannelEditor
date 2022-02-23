<?php
require_once 'starnet_vod_category.php';
require_once 'starnet_vod_list_screen.php';

class StarnetVodCategoryListScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'vod_category_list';

    private $category_list;
    private $category_index;

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_VOD_CATEGORY_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_vod_support()) {
            $plugin->create_screen($this);
        }
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
        return array(GUI_EVENT_KEY_ENTER => ActionFactory::open_folder());
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Vod favorites: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        return null;
    }

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        if (is_null($this->category_index) || is_null($this->category_list)) {
            $this->plugin->config->fetch_vod_categories($plugin_cookies, $this->category_list, $this->category_index);
        }

        $category_list = $this->category_list;

        if (isset($media_url->category_id)) {
            if (!isset($this->category_index[$media_url->category_id])) {
                hd_print("Error: parent category (id: $media_url->category_id) not found.");
                throw new Exception('No parent category found');
            }

            $parent_category = $this->category_index[$media_url->category_id];
            $category_list = $parent_category->get_sub_categories();
        }

        $items = array();

        // Favorites
        if (!isset($media_url->category_id) && $this->plugin->config->get_vod_fav_support()) {
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

        // Filter
        if (!isset($media_url->category_id) && $this->plugin->config->get_vod_portal_support()) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => 'filter_screen',
                PluginRegularFolderItem::caption => DefaultConfig::FILTER_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => DefaultConfig::FILTER_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => 14,
                    ViewItemParams::item_detailed_icon_path => DefaultConfig::FILTER_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        if (!empty($category_list)) {
            foreach ($category_list as $category) {
                $id = $category->get_id();
                if (!is_null($category->get_sub_categories())) {
                    $media_url_str = self::get_media_url_str($id);
                } else if ($id === 'all' || $id === 'search' || $id === 'filter') {
                    $media_url_str = StarnetVodListScreen::get_media_url_str($id, null);
                } else if ($category->get_parent() !== null) {
                    $media_url_str = StarnetVodListScreen::get_media_url_str($category->get_parent()->get_id(), $id);
                } else {
                    $media_url_str = StarnetVodListScreen::get_media_url_str($id, null);
                }

                $items[] = array
                (
                    PluginRegularFolderItem::media_url => $media_url_str,
                    PluginRegularFolderItem::caption => $category->get_caption(),
                    PluginRegularFolderItem::view_item_params => array
                    (
                        ViewItemParams::icon_path => $category->get_icon_path(),
                        ViewItemParams::item_detailed_icon_path => $category->get_icon_path(),
                    )
                );
            }
        }

        return $items;
    }
}

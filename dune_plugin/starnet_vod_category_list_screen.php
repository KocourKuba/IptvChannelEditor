<?php
require_once 'starnet_vod_category.php';
require_once 'starnet_vod_list_screen.php';

class Starnet_Vod_Category_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_category_list';

    /**
     * @var array
     */
    private $category_list;

    /**
     * @var array
     */
    private $category_index;

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_VOD_CATEGORY_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_feature(VOD_MOVIE_PAGE_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @param string $category_id
     * @return false|string
     */
    public static function get_media_url_str($category_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'category_id' => $category_id,
            ));
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(GUI_EVENT_KEY_ENTER => Action_Factory::open_folder());
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Vod favorites: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
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
        if (!isset($media_url->category_id) && $this->plugin->config->get_feature(VOD_FAVORITES_SUPPORTED)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => Vod_Favorites_Screen::get_media_url_str(),
                PluginRegularFolderItem::caption => Default_Config::FAV_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Config::FAV_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => 4,
                    ViewItemParams::item_detailed_icon_path => Default_Config::FAV_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        // Search
        $items[] = array
        (
            PluginRegularFolderItem::media_url => 'search_screen',
            PluginRegularFolderItem::caption => Default_Config::SEARCH_MOVIES_CATEGORY_CAPTION,
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => Default_Config::SEARCH_MOVIES_CATEGORY_ICON_PATH,
                ViewItemParams::item_caption_color => 14,
                ViewItemParams::item_detailed_icon_path => Default_Config::SEARCH_MOVIES_CATEGORY_ICON_PATH,
            )
        );

        // Filter
        if (!isset($media_url->category_id) && $this->plugin->config->get_feature(VOD_PORTAL_SUPPORTED)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => 'filter_screen',
                PluginRegularFolderItem::caption => Default_Config::FILTER_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Config::FILTER_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => 14,
                    ViewItemParams::item_detailed_icon_path => Default_Config::FILTER_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        if (!empty($category_list)) {
            foreach ($category_list as $category) {
                $id = $category->get_id();
                if (!is_null($category->get_sub_categories())) {
                    $media_url_str = self::get_media_url_str($id);
                } else if ($id === 'all' || $id === 'search' || $id === 'filter') {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($id, null);
                } else if ($category->get_parent() !== null) {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($category->get_parent()->get_id(), $id);
                } else {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($id, null);
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

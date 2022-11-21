<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod_category.php';
require_once 'starnet_vod_list_screen.php';

class Starnet_Vod_Category_List_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_category_list';
    const ACTION_RELOAD = 'reload';

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
        parent::__construct(self::ID, $plugin, $plugin->GET_VOD_CATEGORY_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
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
        $actions = array();
        $actions[GUI_EVENT_KEY_ENTER] = Action_Factory::open_folder();
        $reload = User_Input_Handler_Registry::create_action($this, self::ACTION_RELOAD);
        $reload['caption'] = 'Перечитать плейлист';
        $actions[GUI_EVENT_KEY_C_YELLOW] = $reload;

        return $actions;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return null
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Vod favorites: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        if ($user_input->control_id === self::ACTION_RELOAD) {
            hd_print("reload categories");
            $this->clear_vod();
            $media_url = MediaURL::decode($user_input->parent_media_url);
            $range = $this->get_folder_range($media_url, 0, $plugin_cookies);
            return Action_Factory::update_regular_folder($range, true, -1);
        }
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
        //hd_print("Starnet_Vod_Category_List_Screen: get_all_folder_items");
        if (is_null($this->category_index) || is_null($this->category_list)) {
            $this->plugin->config->fetchVodCategories($plugin_cookies, $this->category_list, $this->category_index);
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
        if (!isset($media_url->category_id)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Favorites_Screen::ID,
                PluginRegularFolderItem::caption => Default_Dune_Plugin::FAV_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Dune_Plugin::FAV_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GOLD, // Light yellow
                    ViewItemParams::item_detailed_icon_path => Default_Dune_Plugin::FAV_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        // History
        $items[] = array
        (
            PluginRegularFolderItem::media_url => Starnet_Vod_History_Screen::ID,
            PluginRegularFolderItem::caption => Default_Dune_Plugin::HISTORY_MOVIES_CATEGORY_CAPTION,
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => Default_Dune_Plugin::HISTORY_MOVIES_CATEGORY_ICON_PATH,
                ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_TURQUOISE, // Cyan
                ViewItemParams::item_detailed_icon_path => Default_Dune_Plugin::HISTORY_MOVIES_CATEGORY_ICON_PATH,
            )
        );

        // Search
        $items[] = array
        (
            PluginRegularFolderItem::media_url => Starnet_Vod_Search_Screen::ID,
            PluginRegularFolderItem::caption => Default_Dune_Plugin::SEARCH_MOVIES_CATEGORY_CAPTION,
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => Default_Dune_Plugin::SEARCH_MOVIES_CATEGORY_ICON_PATH,
                ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GREEN, // Green
                ViewItemParams::item_detailed_icon_path => Default_Dune_Plugin::SEARCH_MOVIES_CATEGORY_ICON_PATH,
            )
        );

        // Filter
        if (!isset($media_url->category_id) && $this->plugin->config->get_feature(Plugin_Constants::VOD_FILTER_SUPPORTED)) {
            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Filter_Screen::ID,
                PluginRegularFolderItem::caption => Default_Dune_Plugin::FILTER_MOVIES_CATEGORY_CAPTION,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => Default_Dune_Plugin::FILTER_MOVIES_CATEGORY_ICON_PATH,
                    ViewItemParams::item_caption_color => DEF_LABEL_TEXT_COLOR_GREEN, // Green
                    ViewItemParams::item_detailed_icon_path => Default_Dune_Plugin::FILTER_MOVIES_CATEGORY_ICON_PATH,
                )
            );
        }

        if (!empty($category_list)) {
            foreach ($category_list as $category) {
                $category_id = $category->get_id();
                if (!is_null($category->get_sub_categories())) {
                    $media_url_str = self::get_media_url_str($category_id);
                } else if ($category_id === Vod_Category::FLAG_ALL
                    || $category_id === Vod_Category::FLAG_SEARCH
                    || $category_id === Vod_Category::FLAG_FILTER) {
                    // special category id's
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($category_id, null);
                } else if ($category->get_parent() !== null) {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($category->get_parent()->get_id(), $category_id);
                } else {
                    $media_url_str = Starnet_Vod_List_Screen::get_media_url_str($category_id, null);
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

    public function clear_vod()
    {
        $this->category_list = null;
        $this->category_index = null;
        $this->plugin->vod->clear_movie_cache();
        $this->plugin->config->ClearVodCache();
    }
}

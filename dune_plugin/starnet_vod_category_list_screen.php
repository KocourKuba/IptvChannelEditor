﻿<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod_category.php';
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
        parent::__construct(self::ID, $plugin, $plugin->GET_VOD_CATEGORY_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_feature(VOD_SUPPORTED)) {
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
        $setup_screen = Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина');
        return array(
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_SETUP => $setup_screen,
            GUI_EVENT_KEY_B_GREEN => $setup_screen,
        );
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
        //hd_print('Vod favorites: handle_user_input:');
        //foreach($user_input as $key => $value) hd_print("  $key => $value");

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
        if (!isset($media_url->category_id) && $this->plugin->config->get_feature(VOD_FILTER_SUPPORTED)) {
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
                } else if ($category_id === Vod_Category::PATTERN_ALL
                    || $category_id === Vod_Category::PATTERN_SEARCH
                    || $category_id === Vod_Category::PATTERN_FILTER) {
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
}

<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Search_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'search_screen';
    const SEARCH_ICON_PATH = 'plugin_file://img/icon_search.png';

    const VOD_SEARCH_LIST = 'vod_search_items';
    const VOD_SEARCH_ITEM = 'vod_search_item';

    /**
     * @param string $category
     * @return false|string
     */
    public static function get_media_url_string($category = '')
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'category' => $category));
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions[GUI_EVENT_KEY_ENTER] = User_Input_Handler_Registry::create_action($this,
        ACTION_CREATE_SEARCH, null, array(ACTION_SEARCH => ACTION_OPEN_FOLDER));

        $actions[GUI_EVENT_KEY_B_GREEN] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up'));
        $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down'));
        $actions[GUI_EVENT_KEY_D_BLUE] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete'));
        $actions[GUI_EVENT_KEY_POPUP_MENU] = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU);

        return $actions;
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        switch ($user_input->control_id) {
            case ACTION_CREATE_SEARCH:
                if (!isset($user_input->parent_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== Vod_Category::FLAG_SEARCH && $user_input->{ACTION_SEARCH} === ACTION_OPEN_FOLDER) {
                    return Action_Factory::open_folder($user_input->selected_media_url);
                }

                if ($user_input->{ACTION_SEARCH} === ACTION_ITEMS_EDIT) {
                    $search_string = $media_url->genre_id;
                } else {
                    $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                    $search_string = $search_items === null ? "" : $search_items[0];
                }

                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null, ACTION_NEW_SEARCH, '',
                    $search_string, false, false, true, true, 1300, false, true);
                Control_Factory::add_vgap($defs, 500);

                return Action_Factory::show_dialog(TR::t('search'), $defs, true);

            case ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(
                    User_Input_Handler_Registry::create_action($this, ACTION_RUN_SEARCH));

            case ACTION_RUN_SEARCH:
                $search_string = $user_input->{ACTION_NEW_SEARCH};
                hd_debug_print("search string: $search_string");
                $search_items = new Ordered_Array(HD::get_data_items(self::VOD_SEARCH_LIST));
                $search_items->insert_item($search_string, false);
                HD::put_data_items(self::VOD_SEARCH_LIST, $search_items->get_order());
                $action = Action_Factory::open_folder(
                    Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_SEARCH, $search_string),
                    TR::t('search__1', ": $search_string"));

                return Action_Factory::invalidate_folders(array($user_input->parent_media_url), $action);

            case ACTION_ITEM_UP:
            case ACTION_ITEM_DOWN:
            case ACTION_ITEM_DELETE:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $filter_items = new Ordered_Array(HD::get_data_items(self::VOD_SEARCH_LIST));

                switch ($user_input->control_id) {
                    case ACTION_ITEM_UP:
                        $user_input->sel_ndx--;
                        $filter_items->arrange_item($media_url->genre_id, Ordered_Array::UP);
                        break;

                    case ACTION_ITEM_DOWN:
                        $user_input->sel_ndx++;
                        $filter_items->arrange_item($media_url->genre_id, Ordered_Array::DOWN);
                        break;

                    case ACTION_ITEM_DELETE:
                        $filter_items->remove_item($media_url->genre_id);
                        break;
                }

                HD::put_data_items(self::VOD_SEARCH_LIST, $filter_items->get_order());
                return Action_Factory::invalidate_folders(array(self::get_media_url_str()));

            case GUI_EVENT_KEY_POPUP_MENU:
                if (isset($user_input->selected_media_url)
                    && MediaURL::decode($user_input->selected_media_url)->genre_id !== Vod_Category::FLAG_SEARCH) {

                    $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_EDIT, TR::t('edit'), "edit.png");
                    return Action_Factory::show_popup_menu($menu_items);
                }
                break;

            case ACTION_ITEMS_EDIT:
                return User_Input_Handler_Registry::create_action($this,
                    ACTION_CREATE_SEARCH,
                    null,
                    array(ACTION_SEARCH => ACTION_ITEMS_EDIT)
                );
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        hd_debug_print(null, true);

        $items[] = array(
            PluginRegularFolderItem::caption => TR::t('new_search'),
            PluginRegularFolderItem::view_item_params => array(
                ViewItemParams::icon_path => self::SEARCH_ICON_PATH,
                ViewItemParams::item_layout => HALIGN_LEFT,
                ViewItemParams::icon_valign => VALIGN_CENTER,
                ViewItemParams::icon_dx => 20,
                ViewItemParams::icon_dy => -5,
                ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                ViewItemParams::item_caption_width => 1100
            ),
            PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_SEARCH, Vod_Category::FLAG_SEARCH)
        );

        $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
        foreach ($search_items as $item) {
            if (!empty($item)) {
                $items[] = array(
                    PluginRegularFolderItem::caption => TR::t('search__1', ": $item"),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => self::SEARCH_ICON_PATH,
                        ViewItemParams::item_layout => HALIGN_LEFT,
                        ViewItemParams::icon_valign => VALIGN_CENTER,
                        ViewItemParams::icon_dx => 20,
                        ViewItemParams::icon_dy => -5,
                        ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                        ViewItemParams::item_caption_width => 1100
                    ),
                    PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_SEARCH, $item)
                );
            }
        }
        return $items;
    }

    /**
     * @inheritDoc
     */
    public function get_folder_views()
    {
        hd_debug_print(null, true);

        return array(
            $this->plugin->get_screen_view('list_1x11_info'),
            $this->plugin->get_screen_view('list_1x11_small_info'),
        );
    }
}

<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Filter_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'filter_screen';
    const FILTER_ICON_PATH = 'plugin_file://img/icon_filter.png';

    const VOD_FILTER_LIST = 'vod_filter_items';
    const VOD_FILTER_ITEM = 'vod_filter_item';

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
        return array(
            GUI_EVENT_KEY_ENTER      => User_Input_Handler_Registry::create_action($this,
                ACTION_CREATE_FILTER, null, array(ACTION_FILTER => ACTION_OPEN_FOLDER)),
            GUI_EVENT_KEY_B_GREEN    => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up')),
            GUI_EVENT_KEY_C_YELLOW   => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down')),
            GUI_EVENT_KEY_D_BLUE     => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete')),
            GUI_EVENT_KEY_POPUP_MENU => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_KEY_RETURN     => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_RETURN),
            GUI_EVENT_KEY_STOP       => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_STOP),
        );
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        switch ($user_input->control_id) {
            case GUI_EVENT_KEY_RETURN:
                return Action_Factory::close_and_run();

            case GUI_EVENT_KEY_POPUP_MENU:
                if (isset($user_input->selected_media_url)
                    && MediaURL::decode($user_input->selected_media_url)->genre_id !== Vod_Category::FLAG_FILTER) {

                    $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_EDIT, TR::t('edit'), "edit.png");
                    return Action_Factory::show_popup_menu($menu_items);
                }

                break;

            case ACTION_CREATE_FILTER:
                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== Vod_Category::FLAG_FILTER && $user_input->{ACTION_FILTER} === ACTION_OPEN_FOLDER) {
                    return Action_Factory::open_folder($user_input->selected_media_url);
                }

                return $this->plugin->config->AddFilterUI($this, $media_url->genre_id);

            case ACTION_RUN_FILTER:
                $filter_string = $this->plugin->config->CompileSaveFilterItem($user_input);
                if (empty($filter_string)) break;

                hd_debug_print("filter_screen filter string: $filter_string");
                $filter_items = new Ordered_Array(HD::get_data_items(self::VOD_FILTER_LIST));
                if (!empty($user_input->{ACTION_ITEMS_EDIT}) && (int)$user_input->{ACTION_ITEMS_EDIT} !== -1) {
                    $filter_items->set_item_by_idx($user_input->{ACTION_ITEMS_EDIT}, $filter_string);
                } else {
                    $filter_items->add_item($filter_string);
                }

                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items->get_order());
                return Action_Factory::invalidate_folders(
                    array($user_input->parent_media_url),
                    Action_Factory::open_folder(
                        Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_FILTER, $filter_string),
                        TR::t('filter')
                    )
                );

            case ACTION_ITEMS_EDIT:
                return User_Input_Handler_Registry::create_action($this,
                    ACTION_CREATE_FILTER,
                    null,
                    array(ACTION_FILTER => ACTION_ITEMS_EDIT)
                );

            case ACTION_ITEM_UP:
            case ACTION_ITEM_DOWN:
            case ACTION_ITEM_DELETE:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $filter_items = new Ordered_Array(HD::get_data_items(self::VOD_FILTER_LIST));

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

                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items->get_order());
                return Action_Factory::invalidate_folders(array(self::get_media_url_str()));
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
            PluginRegularFolderItem::caption => TR::t('vod_screen_new_filter'),
            PluginRegularFolderItem::view_item_params => array(
                ViewItemParams::icon_path => self::FILTER_ICON_PATH,
                ViewItemParams::item_layout => HALIGN_LEFT,
                ViewItemParams::icon_valign => VALIGN_CENTER,
                ViewItemParams::icon_dx => 20,
                ViewItemParams::icon_dy => -5,
                ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                ViewItemParams::item_caption_width => 1100
            ),
            PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_FILTER, Vod_Category::FLAG_FILTER)
        );

        $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
        foreach ($filter_items as $item) {
            if (!empty($item)) {
                $items[] = array(
                    PluginRegularFolderItem::caption => TR::t('filter__1', $item),
                    PluginRegularFolderItem::view_item_params => array(
                        ViewItemParams::icon_path => self::FILTER_ICON_PATH,
                        ViewItemParams::item_layout => HALIGN_LEFT,
                        ViewItemParams::icon_valign => VALIGN_CENTER,
                        ViewItemParams::icon_dx => 20,
                        ViewItemParams::icon_dy => -5,
                        ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                        ViewItemParams::item_caption_width => 1100
                    ),
                    PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_FILTER, $item)
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
            $this->plugin->get_screen_view('list_1x11_small_info'),
            $this->plugin->get_screen_view('list_1x11_normal_info'),
        );
    }
}

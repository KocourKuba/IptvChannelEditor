<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class Starnet_Vod_Filter_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'filter_screen';
    const FILTER_ICON_PATH = 'plugin_file://icons/icon_filter.png';

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
        $actions = array();
        $add_params['filter_actions'] = 'open';
        $actions[GUI_EVENT_KEY_ENTER] = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_FILTER, null, $add_params);

        $add_params['filter_actions'] = 'keyboard';
        $actions[GUI_EVENT_KEY_PLAY] = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_FILTER, null, $add_params);

        $actions[GUI_EVENT_KEY_B_GREEN] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up'));
        $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down'));
        $actions[GUI_EVENT_KEY_D_BLUE] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete'));
        $actions[GUI_EVENT_KEY_POPUP_MENU] = Action_Factory::show_popup_menu(array());

        return $actions;
    }

    /**
     * @param $user_input
     * @param $sel_inc
     * @param &$plugin_cookies
     * @return array
     */
    private function get_update_action($user_input, $sel_inc, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $range = $this->create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

        return Action_Factory::update_regular_folder($range, false, $user_input->sel_ndx + $sel_inc);
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        switch ($user_input->control_id) {
            case ACTION_CREATE_FILTER:
                if (!isset($user_input->parent_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== Vod_Category::FLAG_FILTER && $user_input->filter_actions !== 'keyboard') {
                    return Action_Factory::open_folder($user_input->selected_media_url);
                }

                if ($user_input->filter_actions === 'keyboard') {
                    $filter_string = $media_url->genre_id;
                } else {
                    $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
                    $filter_string = empty($filter_items) ? "" : $filter_items[0];
                }

                $defs = array();
                if (false === $this->plugin->config->AddFilterUI($defs, $this, $filter_string)) break;

                Control_Factory::add_close_dialog_and_apply_button($defs, $this, null, ACTION_RUN_FILTER, TR::t('ok'), 300);
                Control_Factory::add_close_dialog_button($defs, TR::t('cancel'), 300);
                Control_Factory::add_vgap($defs, 10);

                return Action_Factory::show_dialog(TR::t('filter'), $defs, true);

            case ACTION_RUN_FILTER:
                $filter_string = $this->plugin->config->CompileSaveFilterItem($user_input);
                if (empty($filter_string)) break;

                hd_debug_print("filter_screen filter string: $filter_string");
                $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
                $i = array_search($filter_string, $filter_items);
                if ($i !== false) {
                    unset ($filter_items [$i]);
                }
                array_unshift($filter_items, $filter_string);
                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items);

                return Action_Factory::invalidate_folders(array(self::ID),
                    Action_Factory::open_folder(
                        Starnet_Vod_List_Screen::get_media_url_string(Vod_Category::FLAG_FILTER, $filter_string),
                        TR::t('filter__1', $filter_string)));

            case ACTION_ITEM_UP:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
                $i = array_search($video_id, $filter_items);
                if ($i === false || $i === 0) break;

                $t = $filter_items[$i - 1];
                $filter_items[$i - 1] = $filter_items[$i];
                $filter_items[$i] = $t;
                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items);

                return $this->get_update_action($user_input, -1, $plugin_cookies);

            case ACTION_ITEM_DOWN:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
                $i = array_search($video_id, $filter_items);
                if ($i === false || $i === count($filter_items) - 1) break;

                $t = $filter_items[$i + 1];
                $filter_items[$i + 1] = $filter_items[$i];
                $filter_items[$i] = $t;
                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items);

                return $this->get_update_action($user_input, 1, $plugin_cookies);

            case ACTION_ITEM_DELETE:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_data_items(self::VOD_FILTER_LIST);
                $i = array_search($video_id, $filter_items);
                if ($i !== false) {
                    unset ($filter_items[$i]);
                }
                HD::put_data_items(self::VOD_FILTER_LIST, $filter_items);

                return Action_Factory::invalidate_folders(array(self::ID));
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
        $items = array();

        $items[] = array
        (
            PluginRegularFolderItem::caption => TR::t('vod_screen_new_filter'),
            PluginRegularFolderItem::view_item_params => array
            (
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
                $items[] = array
                (
                    PluginRegularFolderItem::caption => TR::t('filter__1', $item),
                    PluginRegularFolderItem::view_item_params => array
                    (
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
            $this->plugin->get_screen_view('list_1x11_info'),
        );
    }
}

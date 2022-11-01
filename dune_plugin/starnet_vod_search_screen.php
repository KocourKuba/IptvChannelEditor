<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_Vod_Search_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'search_screen';
    const SEARCH_ICON_PATH = 'plugin_file://icons/icon_search.png';

    const ACTION_CREATE_SEARCH = 'create_search';
    const ACTION_NEW_SEARCH = 'new_search';
    const ACTION_RUN_SEARCH = 'run_search';
    const ACTION_ITEM_UP = 'item_up';
    const ACTION_ITEM_DOWN = 'item_down';
    const ACTION_ITEM_DELETE = 'item_delete';

    const SEARCH_LIST = 'search_items';
    const SEARCH_ITEM = 'search_item';

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_search_folder_views());

        if ($plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @param string $category
     * @return false|string
     */
    public static function get_media_url_str($category = '')
    {
        return MediaURL::encode(array('screen_id' => self::ID, 'category' => $category));
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();
        $add_params['search_actions'] = 'open';
        $actions[GUI_EVENT_KEY_ENTER] = User_Input_Handler_Registry::create_action($this, self::ACTION_CREATE_SEARCH, $add_params);

        $add_params['search_actions'] = 'keyboard';
        $actions[GUI_EVENT_KEY_PLAY] = User_Input_Handler_Registry::create_action($this, self::ACTION_CREATE_SEARCH, $add_params);

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_UP);
        $add_action['caption'] = 'Вверх';
        $actions[GUI_EVENT_KEY_B_GREEN] = $add_action;

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_DOWN);
        $add_action['caption'] = 'Вниз';
        $actions[GUI_EVENT_KEY_C_YELLOW] = $add_action;

        $add_action = User_Input_Handler_Registry::create_action($this, self::ACTION_ITEM_DELETE);
        $add_action['caption'] = 'Удалить';
        $actions[GUI_EVENT_KEY_D_BLUE] = $add_action;

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
        $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

        return Action_Factory::update_regular_folder($range, false, $user_input->sel_ndx + $sel_inc);
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_print('StarnetSearchScreen::handle_user_input:');
        foreach($user_input as $key => $value) hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case self::ACTION_CREATE_SEARCH:
                if (!isset($user_input->parent_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== Vod_Category::PATTERN_SEARCH && $user_input->search_actions !== 'keyboard') {
                    return Action_Factory::open_folder($user_input->selected_media_url);
                }

                if ($user_input->search_actions === 'keyboard') {
                    $search_string = $media_url->genre_id;
                } else {
                    $search_string = HD::get_item(self::SEARCH_ITEM);
                }

                $defs = array();
                Control_Factory::add_text_field($defs,
                    $this, null, self::ACTION_NEW_SEARCH, '',
                    $search_string, false, false, true, true, 1300, false, true);
                Control_Factory::add_vgap($defs, 500);

                return Action_Factory::show_dialog('Поиск', $defs, true);

            case self::ACTION_NEW_SEARCH:
                return Action_Factory::close_dialog_and_run(
                    User_Input_Handler_Registry::create_action($this, self::ACTION_RUN_SEARCH));

            case self::ACTION_RUN_SEARCH:
                $search_string = $user_input->{self::ACTION_NEW_SEARCH};
                hd_print("search string: $search_string");
                HD::put_item(self::SEARCH_ITEM, $search_string);
                $search_items = HD::get_items(self::SEARCH_LIST);
                $i = array_search($search_string, $search_items);
                if ($i !== false) {
                    unset ($search_items [$i]);
                }
                array_unshift($search_items, $search_string);
                HD::put_items(self::SEARCH_LIST, $search_items);
                $action = Action_Factory::open_folder(
                    Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::PATTERN_SEARCH, $search_string),
                    "Поиск: " . $search_string);

                return Action_Factory::invalidate_folders(array(self::ID), $action);

            case self::ACTION_ITEM_UP:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items(self::SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i === false || $i === 0)  break;

                $t = $search_items[$i - 1];
                $search_items[$i - 1] = $search_items[$i];
                $search_items[$i] = $t;
                HD::put_items(self::SEARCH_LIST, $search_items);

                return $this->get_update_action($user_input, -1, $plugin_cookies);

            case self::ACTION_ITEM_DOWN:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items(self::SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i === false || $i === count($search_items) - 1) break;

                $t = $search_items[$i + 1];
                $search_items[$i + 1] = $search_items[$i];
                $search_items[$i] = $t;
                HD::put_items(self::SEARCH_LIST, $search_items);

                return $this->get_update_action($user_input, 1, $plugin_cookies);

            case self::ACTION_ITEM_DELETE:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items(self::SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i !== false) {
                    unset ($search_items[$i]);
                }
                HD::put_items(self::SEARCH_LIST, $search_items);

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
        hd_print("Starnet_Search_Screen::get_all_folder_items");
        $items = array();

        $items[] = array
        (
            PluginRegularFolderItem::caption => '[Новый поиск]',
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => self::SEARCH_ICON_PATH,
                ViewItemParams::item_layout => HALIGN_LEFT,
                ViewItemParams::icon_valign => VALIGN_CENTER,
                ViewItemParams::icon_dx => 20,
                ViewItemParams::icon_dy => -5,
                ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                ViewItemParams::item_caption_width => 1100
            ),
            PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::PATTERN_SEARCH, Vod_Category::PATTERN_SEARCH)
        );

        $search_items = HD::get_items(self::SEARCH_LIST);
        foreach ($search_items as $item) {
            if (!empty($item)) {
                $items[] = array
                (
                    PluginRegularFolderItem::caption => "Поиск: $item",
                    PluginRegularFolderItem::view_item_params => array
                    (
                        ViewItemParams::icon_path => self::SEARCH_ICON_PATH,
                        ViewItemParams::item_layout => HALIGN_LEFT,
                        ViewItemParams::icon_valign => VALIGN_CENTER,
                        ViewItemParams::icon_dx => 20,
                        ViewItemParams::icon_dy => -5,
                        ViewItemParams::item_caption_font_size => FONT_SIZE_NORMAL,
                        ViewItemParams::item_caption_width => 1100
                    ),
                    PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::PATTERN_SEARCH, $item)
                );
            }
        }
        return $items;
    }

    /**
     * @param MediaURL $media_url
     * @return null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}


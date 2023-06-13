<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_Vod_Search_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'search_screen';
    const SEARCH_ICON_PATH = 'plugin_file://icons/icon_search.png';

    const VOD_SEARCH_LIST = 'vod_search_items';
    const VOD_SEARCH_ITEM = 'vod_search_item';

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
        $actions[GUI_EVENT_KEY_ENTER] = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, null, $add_params);

        $add_params['search_actions'] = 'keyboard';

        $actions[GUI_EVENT_KEY_PLAY] = User_Input_Handler_Registry::create_action($this, ACTION_CREATE_SEARCH, null, $add_params);
        $actions[GUI_EVENT_KEY_B_GREEN] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_UP, TR::t('up'));
        $actions[GUI_EVENT_KEY_C_YELLOW] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DOWN, TR::t('down'));
        $actions[GUI_EVENT_KEY_D_BLUE] = User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete'));

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
        //dump_input_handler(__METHOD__, $user_input);

        switch ($user_input->control_id) {
            case ACTION_CREATE_SEARCH:
                if (!isset($user_input->parent_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== Vod_Category::FLAG_SEARCH && $user_input->search_actions !== 'keyboard') {
                    return Action_Factory::open_folder($user_input->selected_media_url);
                }

                if ($user_input->search_actions === 'keyboard') {
                    $search_string = $media_url->genre_id;
                } else {
                    $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                    $search_string = empty($search_items) ? "" : $search_items[0];
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
                hd_print(__METHOD__ . ": search string: $search_string");
                $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                $i = array_search($search_string, $search_items);
                if ($i !== false) {
                    unset ($search_items [$i]);
                }
                array_unshift($search_items, $search_string);
                HD::put_data_items(self::VOD_SEARCH_LIST, $search_items);
                $action = Action_Factory::open_folder(
                    Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::FLAG_SEARCH, $search_string),
                    TR::t('search__1', ": $search_string"));

                return Action_Factory::invalidate_folders(array(self::ID), $action);

            case ACTION_ITEM_UP:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i === false || $i === 0)  break;

                $t = $search_items[$i - 1];
                $search_items[$i - 1] = $search_items[$i];
                $search_items[$i] = $t;
                HD::put_data_items(self::VOD_SEARCH_LIST, $search_items);

                return $this->get_update_action($user_input, -1, $plugin_cookies);

            case ACTION_ITEM_DOWN:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i === false || $i === count($search_items) - 1) break;

                $t = $search_items[$i + 1];
                $search_items[$i + 1] = $search_items[$i];
                $search_items[$i] = $t;
                HD::put_data_items(self::VOD_SEARCH_LIST, $search_items);

                return $this->get_update_action($user_input, 1, $plugin_cookies);

            case ACTION_ITEM_DELETE:
                if (!isset($user_input->selected_media_url)) break;

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
                $i = array_search($video_id, $search_items);
                if ($i !== false) {
                    unset ($search_items[$i]);
                }
                HD::put_data_items(self::VOD_SEARCH_LIST, $search_items);

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
        hd_print(__METHOD__);
        $items = array();

        $items[] = array
        (
            PluginRegularFolderItem::caption => TR::t('new_search'),
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
            PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::FLAG_SEARCH, Vod_Category::FLAG_SEARCH)
        );

        $search_items = HD::get_data_items(self::VOD_SEARCH_LIST);
        foreach ($search_items as $item) {
            if (!empty($item)) {
                $items[] = array
                (
                    PluginRegularFolderItem::caption => TR::t('search__1', ": $item"),
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
                    PluginRegularFolderItem::media_url => Starnet_Vod_List_Screen::get_media_url_str(Vod_Category::FLAG_SEARCH, $item)
                );
            }
        }
        return $items;
    }
}

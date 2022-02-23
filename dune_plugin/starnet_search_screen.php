<?php
require_once 'lib/vod/vod.php';
require_once 'lib/abstract_preloaded_regular_screen.php';

class StarnetSearchScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'search_screen';
    const SEARCH_ICON_PATH = 'plugin_file://icons/icon_search.png';

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_search_folder_views());

        if ($plugin->config->get_vod_support()) {
            $plugin->create_screen($this);
        }
    }

    public static function get_media_url_str($category = '')
    {
        return MediaURL::encode
        (
            array
            (
                'screen_id' => self::ID,
                'category' => $category
            )
        );
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();
        $add_params['search_actions'] = 'open';
        $actions[GUI_EVENT_KEY_ENTER] = UserInputHandlerRegistry::create_action($this, 'search', $add_params);

        $add_params['search_actions'] = 'keyboard';
        $actions[GUI_EVENT_KEY_PLAY] = UserInputHandlerRegistry::create_action($this, 'search', $add_params);

        $add_action = UserInputHandlerRegistry::create_action($this, 'item_up');
        $add_action['caption'] = 'Вверх';
        $actions[GUI_EVENT_KEY_B_GREEN] = $add_action;

        $add_action = UserInputHandlerRegistry::create_action($this, 'item_down');
        $add_action['caption'] = 'Вниз';
        $actions[GUI_EVENT_KEY_C_YELLOW] = $add_action;

        $add_action = UserInputHandlerRegistry::create_action($this, 'delete');
        $add_action['caption'] = 'Удалить';
        $actions[GUI_EVENT_KEY_D_BLUE] = $add_action;

        return $actions;
    }

    private function get_update_action(&$user_input, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

        return ActionFactory::update_regular_folder($range, true, 1);
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('StarnetSearchScreen: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        switch ($user_input->control_id) {
            case 'search':
                if (!isset($user_input->parent_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== 'search' && $user_input->search_actions !== 'keyboard') {
                    return ActionFactory::open_folder($user_input->selected_media_url);
                }

                $defs = array();
                if ($user_input->search_actions === 'keyboard') {
                    $search_text = $media_url->genre_id;
                } else {
                    $search_text = HD::get_item('search_item');
                }

                ControlFactory::add_text_field($defs,
                    $this, null,
                    'do_new_search', '',
                    $search_text, 0, 0, 1, 1, 1300, 0, 1);
                ControlFactory::add_vgap($defs, 500);

                return ActionFactory::show_dialog('Поиск', $defs, true);

            case 'do_new_search':
                return ActionFactory::close_dialog_and_run(
                    UserInputHandlerRegistry::create_action($this, 'run_search'));

            case 'run_search':
                HD::put_item('search_item', $user_input->do_new_search);
                $search_items = HD::get_items('search_items');
                $i = array_search($user_input->do_new_search, $search_items);
                if ($i !== false) {
                    unset ($search_items [$i]);
                }
                array_unshift($search_items, $user_input->do_new_search);
                HD::put_items('search_items', $search_items);
                $action = ActionFactory::open_folder(
                    StarnetVodListScreen::get_media_url_str('search', $user_input->do_new_search),
                    "Поиск: " . $user_input->do_new_search);

                return ActionFactory::invalidate_folders(array('search_screen'), $action);

            case 'item_up':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items('search_items');
                $i = array_search($video_id, $search_items);
                if ($i !== false && $i !== 0) {
                    $t = $search_items[$i - 1];
                    $search_items[$i - 1] = $search_items[$i];
                    $search_items[$i] = $t;
                }
                HD::put_items('search_items', $search_items);
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

                return ActionFactory::update_regular_folder($range, false, $user_input->sel_ndx - 1);

            case 'item_down':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items('search_items');
                $i = array_search($video_id, $search_items);
                if ($i !== false && $i !== count($search_items) - 1) {
                    $t = $search_items[$i + 1];
                    $search_items[$i + 1] = $search_items[$i];
                    $search_items[$i] = $t;
                }
                HD::put_items('search_items', $search_items);
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

                return ActionFactory::update_regular_folder($range, false, $user_input->sel_ndx + 1);

            case 'delete':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $search_items = HD::get_items('search_items');
                $i = array_search($video_id, $search_items);
                if ($i !== false) {
                	unset ($search_items[$i]);
				}
                HD::put_items('search_items', $search_items);

                return ActionFactory::invalidate_folders(array('search_screen'));

            case 'popup_menu':
                break;
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);
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
            PluginRegularFolderItem::media_url => $this->plugin->vod->get_search_media_url_str('search')
        );

        $search_items = HD::get_items('search_items');
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
                    PluginRegularFolderItem::media_url => $this->plugin->vod->get_search_media_url_str($item)
                );
            }
        }
        return $items;
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}


<?php
require_once 'lib/vod/vod.php';
require_once 'lib/abstract_preloaded_regular_screen.php';

class StarnetFilterScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'filter_screen';
    const FILTER_ICON_PATH = 'plugin_file://icons/icon_filter.png';
    protected $plugin;

    public function __construct(DefaultDunePlugin $plugin)
    {
        $this->plugin = $plugin;
        parent::__construct(self::ID, $this->plugin->vod->get_vod_search_folder_views());

        if ($this->plugin->config->get_vod_portal_support()) {
            $this->plugin->create_screen($this);
        }

        UserInputHandlerRegistry::get_instance()->register_handler($this);
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

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();
        $add_params['filter_actions'] = 'open';
        $actions[GUI_EVENT_KEY_ENTER] = UserInputHandlerRegistry::create_action($this, 'filter', $add_params);

        $add_params['filter_actions'] = 'keyboard';
        $actions[GUI_EVENT_KEY_PLAY] = UserInputHandlerRegistry::create_action($this, 'filter', $add_params);

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

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    private function get_update_action(&$user_input, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

        return ActionFactory::update_regular_folder($range, true, 1);
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('StarnetFilterScreen: handle_user_input:');
        // foreach ($user_input as $key => $value) {
        //     hd_print("  $key => $value");
        // }

        switch ($user_input->control_id) {
            case 'filter':
                if (!isset($user_input->parent_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                if ($media_url->genre_id !== 'filter' && $user_input->filter_actions !== 'keyboard') {
                    return ActionFactory::open_folder($user_input->selected_media_url);
                }

                if ($user_input->filter_actions === 'keyboard') {
                    $filter_string = $media_url->genre_id;
                } else {
                    $filter_string = HD::get_item('filter_item');
                }

                $defs = array();
                if (false === $this->plugin->config->AddFilterUI($defs, $this, $filter_string)) {
                    return null;
                }

                ControlFactory::add_close_dialog_and_apply_button($defs, $this, null, 'apply_filter', 'Ok', 300);
                ControlFactory::add_close_dialog_button($defs, 'Отмена', 300);
                ControlFactory::add_vgap($defs, 10);

                return ActionFactory::show_dialog('Фильтр', $defs, true);

            case 'apply_filter':
                $filter_string = $this->plugin->config->CompileSaveFilterItem($user_input);
                if (empty($filter_string)) {
                    return null;
                }

                hd_print("filter_screen filter string: $filter_string");
                HD::put_item('filter_item', $filter_string);
                $filter_items = HD::get_items('filter_items');
                $i = array_search($filter_string, $filter_items);
                if ($i !== false) {
                    unset ($filter_items [$i]);
                }
                array_unshift($filter_items, $filter_string);
                HD::put_items('filter_items', $filter_items);

                return ActionFactory::invalidate_folders(array('filter_screen'),
                    ActionFactory::open_folder(
                        StarnetVodListScreen::get_media_url_str('filter', $filter_string),
                        "Фильтр: " . $filter_string));

            case 'item_up':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_items('filter_items');
                $i = array_search($video_id, $filter_items);
                if ($i !== false && $i !== 0) {
                    $t = $filter_items[$i - 1];
                    $filter_items[$i - 1] = $filter_items[$i];
                    $filter_items[$i] = $t;
                }
                HD::put_items('filter_items', $filter_items);
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

                return ActionFactory::update_regular_folder($range, false, $user_input->sel_ndx - 1);

            case 'item_down':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_items('filter_items');
                $i = array_search($video_id, $filter_items);
                if ($i !== false && $i !== count($filter_items) - 1) {
                    $t = $filter_items[$i + 1];
                    $filter_items[$i + 1] = $filter_items[$i];
                    $filter_items[$i] = $t;
                }
                HD::put_items('filter_items', $filter_items);
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));

                return ActionFactory::update_regular_folder($range, false, $user_input->sel_ndx + 1);

            case 'delete':
                if (!isset($user_input->selected_media_url)) {
                    return null;
                }

                $media_url = MediaURL::decode($user_input->selected_media_url);
                $video_id = $media_url->genre_id;
                $filter_items = HD::get_items('filter_items');
                $i = array_search($video_id, $filter_items);
                if ($i !== false) {
                	unset ($filter_items[$i]);
				}
                HD::put_items('filter_items', $filter_items);

                return ActionFactory::invalidate_folders(array('filter_screen'));

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
            PluginRegularFolderItem::caption => '[Новый фильтр]',
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
            PluginRegularFolderItem::media_url => $this->plugin->vod->get_filter_media_url_str('filter')
        );

        $filter_items = HD::get_items('filter_items');
        foreach ($filter_items as $item) {
            if (!empty($item)) {
                $items[] = array
                (
                    PluginRegularFolderItem::caption => "Фильтр: $item",
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
                    PluginRegularFolderItem::media_url => $this->plugin->vod->get_filter_media_url_str($item)
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


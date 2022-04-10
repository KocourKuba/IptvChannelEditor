<?php
require_once 'tv.php';
require_once 'lib/abstract_preloaded_regular_screen.php';

class TvFavoritesScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'tv_favorites';

    public static function get_media_url_str()
    {
        return MediaURL::encode(array(
            'screen_id' => self::ID,
            'is_favorites' => true)
        );
    }

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_TV_CHANNEL_LIST_FOLDER_VIEWS());

        if ($plugin->config->get_tv_fav_support()) {
            $plugin->create_screen($this);
        }
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $move_backward_favorite_action = UserInputHandlerRegistry::create_action($this, 'move_backward_favorite');
        $move_backward_favorite_action['caption'] = 'Вверх';

        $move_forward_favorite_action = UserInputHandlerRegistry::create_action($this, 'move_forward_favorite');
        $move_forward_favorite_action['caption'] = 'Вниз';

        $remove_favorite_action = UserInputHandlerRegistry::create_action($this, 'remove_favorite');
        $remove_favorite_action['caption'] = 'Удалить';

        $remove_all_favorite_action = UserInputHandlerRegistry::create_action($this, 'remove_all_favorite');

        $menu_items = array(
            array(GuiMenuItemDef::caption => 'Удалить из Избранного', GuiMenuItemDef::action => $remove_favorite_action),
            array(GuiMenuItemDef::caption => 'Очистить Избранное', GuiMenuItemDef::action => $remove_all_favorite_action),
        );

        $popup_menu_action = ActionFactory::show_popup_menu($menu_items);

        return array
        (
            GUI_EVENT_KEY_ENTER => ActionFactory::tv_play(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => $move_backward_favorite_action,
            GUI_EVENT_KEY_C_YELLOW => $move_forward_favorite_action,
            GUI_EVENT_KEY_D_BLUE => $remove_favorite_action,
            GUI_EVENT_KEY_POPUP_MENU => $popup_menu_action,
        );
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    private function get_update_action($sel_increment, &$user_input, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);

        $num_favorites = count($this->plugin->tv->get_fav_channel_ids($plugin_cookies));

        $sel_ndx = $user_input->sel_ndx + $sel_increment;
        if ($sel_ndx < 0) {
            $sel_ndx = 0;
        }
        if ($sel_ndx >= $num_favorites) {
            $sel_ndx = $num_favorites - 1;
        }

        $range = HD::create_regular_folder_range($this->get_all_folder_items($parent_media_url, $plugin_cookies));
        return ActionFactory::update_regular_folder($range, true, $sel_ndx);
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Tv favorites: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        switch ($user_input->control_id) {
            case 'move_backward_favorite':
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_UP;
                $inc = -1;
                break;
            case 'move_forward_favorite':
                $fav_op_type = PLUGIN_FAVORITES_OP_MOVE_DOWN;
                $inc = 1;
                break;
            case 'remove_favorite':
                $fav_op_type = PLUGIN_FAVORITES_OP_REMOVE;
                $inc = 0;
                break;
            case 'remove_all_favorite':
                $fav_op_type = 'clear_favorites';
                $inc = 0;
                break;
            default:
                return  null;
        }

        $channel_id = MediaURL::decode($user_input->selected_media_url)->channel_id;
        $this->plugin->tv->change_tv_favorites($fav_op_type, $channel_id, $plugin_cookies);
        return $this->get_update_action($inc, $user_input, $plugin_cookies);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->tv->folder_entered($media_url, $plugin_cookies);

        $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);

        $items = array();

        foreach ($fav_channel_ids as $channel_id) {
            if (!preg_match('/\S/', $channel_id)) {
                continue;
            }

            try {
                $c = $this->plugin->tv->get_channel($channel_id);
            } catch (Exception $e) {
                hd_print("Warning: channel '$channel_id' not found.");
                continue;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => MediaURL::encode(array(
                    'channel_id' => $c->get_id(),
                    'group_id' => '__favorites')
                ),
                PluginRegularFolderItem::caption => $c->get_title(),
                PluginRegularFolderItem::view_item_params => array(
                    ViewItemParams::icon_path => $c->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $c->get_icon_url(),
                ),
                PluginRegularFolderItem::starred => false,
            );
        }

        return $items;
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->tv->get_archive($media_url);
    }
}

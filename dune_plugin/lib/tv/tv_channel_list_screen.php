<?php
require_once 'lib/abstract_preloaded_regular_screen.php';

class TvChannelListScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'tv_channel_list';

    public static function get_media_url_str($group_id)
    {
        return MediaURL::encode(
            array
            (
                'screen_id' => self::ID,
                'group_id' => $group_id,
            ));
    }

    ///////////////////////////////////////////////////////////////////////

    public function __construct(DefaultDunePlugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_TV_CHANNEL_LIST_FOLDER_VIEWS());

        $plugin->create_screen($this);
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();
        $actions[GUI_EVENT_KEY_ENTER] = ActionFactory::tv_play();
        $actions[GUI_EVENT_KEY_PLAY] = ActionFactory::tv_play();
        $actions[GUI_EVENT_KEY_INFO] = UserInputHandlerRegistry::create_action($this, 'info');
        $actions[GUI_EVENT_KEY_B_GREEN] = ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина');

        if ($this->plugin->tv->is_favorites_supported()) {
            $add_favorite_action = UserInputHandlerRegistry::create_action($this, 'add_favorite');
            $add_favorite_action['caption'] = 'В избранное';

            $popup_menu_action = UserInputHandlerRegistry::create_action($this, 'popup_menu');

            $actions[GUI_EVENT_KEY_D_BLUE] = $add_favorite_action;
            $actions[GUI_EVENT_KEY_POPUP_MENU] = $popup_menu_action;
        }

        return $actions;
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    /**
     * @throws Exception
     */
    private function get_sel_item_update_action(&$user_input, &$plugin_cookies)
    {
        $parent_media_url = MediaURL::decode($user_input->parent_media_url);
        $sel_ndx = $user_input->sel_ndx;
        $group = $this->plugin->tv->get_group($parent_media_url->group_id);
        $channels = $group->get_channels($plugin_cookies);

        $items[] = $this->get_regular_folder_item($group, $channels->get_by_ndx($sel_ndx), $plugin_cookies);
        $range = HD::create_regular_folder_range($items, $sel_ndx, $channels->size());

        return ActionFactory::update_regular_folder($range);
    }

    /**
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Tv favorites: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $channel_id = $media_url->channel_id;

        switch ($user_input->control_id) {
            case 'info':
                $c = $this->plugin->tv->get_channels()->get($channel_id);
                $id = $c->get_id();
                $title = $c->get_title();

                return ActionFactory::show_title_dialog("Channel '$title' (id=$id)");
            case 'popup_menu':
                $add_favorite_action = UserInputHandlerRegistry::create_action($this, 'add_favorite');
                $caption = $this->plugin->tv->is_favorite_channel_id($channel_id, $plugin_cookies) ? 'Удалить из Избранного' : 'Добавить в избранное';
                $menu_items[] = array(GuiMenuItemDef::caption => $caption, GuiMenuItemDef::action => $add_favorite_action);

                return ActionFactory::show_popup_menu($menu_items);
            case 'add_favorite':
                $opt_type = $this->plugin->tv->is_favorite_channel_id($channel_id, $plugin_cookies) ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                $this->plugin->tv->change_tv_favorites($opt_type, $channel_id, $plugin_cookies);

                return ActionFactory::invalidate_folders(array(self::get_media_url_str($media_url->group_id)));
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    private function get_regular_folder_item($group, $c, &$plugin_cookies)
    {
        return array
        (
            PluginRegularFolderItem::media_url => MediaURL::encode(array('channel_id' => $c->get_id(), 'group_id' => $group->get_id())),
            PluginRegularFolderItem::caption => $c->get_title(),
            PluginRegularFolderItem::view_item_params => array
            (
                ViewItemParams::icon_path => $c->get_icon_url(),
                ViewItemParams::item_detailed_icon_path => $c->get_icon_url(),
            ),
            PluginRegularFolderItem::starred => $this->plugin->tv->is_favorite_channel_id($c->get_id(), $plugin_cookies),
        );
    }

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->tv->folder_entered($media_url, $plugin_cookies);

        try {
            $this->plugin->tv->ensure_channels_loaded($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Ошибка загрузки плейлиста! " . $e->getMessage());
            return array();
        }

        $group = $this->plugin->tv->get_group($media_url->group_id);

        $items = array();

        foreach ($group->get_channels($plugin_cookies) as $c) {
            $items[] = $this->get_regular_folder_item($group, $c, $plugin_cookies);
        }

        return $items;
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->tv->get_archive($media_url);
    }
}

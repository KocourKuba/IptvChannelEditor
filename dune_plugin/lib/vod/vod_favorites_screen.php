<?php

require_once 'vod.php';
require_once 'lib/abstract_preloaded_regular_screen.php';

class VodFavoritesScreen extends AbstractPreloadedRegularScreen implements UserInputHandler
{
    const ID = 'vod_favorites';

    protected $plugin;

    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    ///////////////////////////////////////////////////////////////////////

    public function __construct(DefaultDunePlugin $plugin)
    {
        $this->plugin = $plugin;

        parent::__construct(self::ID, $this->plugin->vod->get_vod_list_folder_views());

        if ($this->plugin->config->get_vod_support()) {
            $this->plugin->create_screen($this);
            UserInputHandlerRegistry::get_instance()->register_handler($this);
        }
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $actions = array();

        $actions[GUI_EVENT_KEY_ENTER] = $this->plugin->vod->is_movie_page_supported() ? ActionFactory::open_folder() : ActionFactory::vod_play();

        $remove_favorite_action = UserInputHandlerRegistry::create_action($this, 'remove_favorite');
        $remove_favorite_action['caption'] = 'Удалить';

        $menu_items[] = array(
            GuiMenuItemDef::caption => 'Удалить из Избранного',
            GuiMenuItemDef::action => $remove_favorite_action);

        $popup_menu_action = ActionFactory::show_popup_menu($menu_items);

        $actions[GUI_EVENT_KEY_D_BLUE] = $remove_favorite_action;
        $actions[GUI_EVENT_KEY_POPUP_MENU] = $popup_menu_action;

        return $actions;
    }

    public function get_handler_id()
    {
        return self::ID.'_handler';
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Vod favorites: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if ($user_input->control_id === 'remove_favorite') {
            if (!isset($user_input->selected_media_url)) {
                return null;
            }

            $media_url = MediaURL::decode($user_input->selected_media_url);
            $movie_id = $media_url->movie_id;

            $this->plugin->vod->remove_favorite_movie($movie_id, $plugin_cookies);

            return ActionFactory::invalidate_folders(array(self::get_media_url_str()));
        }

        return null;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);

        $this->plugin->vod->ensure_favorites_loaded($plugin_cookies);

        $movie_ids = $this->plugin->vod->get_favorite_movie_ids();

        $items = array();

        foreach ($movie_ids as $movie_id) {
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);
            if (is_null($short_movie)) {
                $caption = "#$movie_id";
                $poster_url = "missing://";
            } else {
                $caption = $short_movie->name;
                $poster_url = $short_movie->poster_url;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url =>
                    VodMovieScreen::get_media_url_str($movie_id),
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $poster_url,
                )
            );
        }

        return $items;
    }

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->vod->get_archive($media_url);
    }
}

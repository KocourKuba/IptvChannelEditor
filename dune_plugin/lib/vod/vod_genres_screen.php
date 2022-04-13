<?php

class Vod_Genres_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_genres';

    /**
     * @return false|string
     */
    public static function get_media_url_str()
    {
        return MediaURL::encode(array('screen_id' => self::ID));
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_genres_folder_views());

        $plugin->create_screen($this);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        $select_genre_action =
            User_Input_Handler_Registry::create_action($this, 'select_genre');

        return array
        (
            GUI_EVENT_KEY_ENTER => $select_genre_action,
        );
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID . '_handler';
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        // hd_print('Vod genres: handle_user_input:');
        // foreach ($user_input as $key => $value)
        //     hd_print("  $key => $value");

        if ($user_input->control_id === 'select_genre') {
            if (!isset($user_input->selected_media_url)) {
                return null;
            }

            $media_url = MediaURL::decode($user_input->selected_media_url);
            $genre_id = $media_url->genre_id;
            $caption = $this->plugin->vod->get_genre_caption($genre_id);
            $media_url_str = $this->plugin->vod->get_genre_media_url_str($genre_id);

            return Action_Factory::open_folder($media_url_str, $caption);
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
        $this->plugin->vod->folder_entered($media_url, $plugin_cookies);

        $this->plugin->vod->ensure_genres_loaded($plugin_cookies);

        $genre_ids = $this->plugin->vod->get_genre_ids();

        $items = array();

        foreach ($genre_ids as $genre_id) {
            $caption = $this->plugin->vod->get_genre_caption($genre_id);
            $media_url_str = $this->plugin->vod->get_genre_media_url_str($genre_id);
            $icon_url = $this->plugin->vod->get_genre_icon_url($genre_id);

            $items[] = array
            (
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $icon_url,
                )
            );
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

<?php

require_once 'lib/abstract_preloaded_regular_screen.php';

class Tv_Group_List_Screen extends Abstract_Preloaded_Regular_Screen
{
    const ID = 'tv_group_list';

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param Default_Dune_Plugin $plugin
     */
    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin, $plugin->config->GET_TV_GROUP_LIST_FOLDER_VIEWS());
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER => Action_Factory::open_folder(),
            GUI_EVENT_KEY_PLAY => Action_Factory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => Action_Factory::open_folder(Starnet_Setup_Screen::get_media_url_str(), 'Настройки плагина'),
        );
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        $this->plugin->tv->folder_entered($media_url, $plugin_cookies);

        try {
            $this->plugin->tv->ensure_channels_loaded($plugin_cookies);
        } catch (Exception $e) {
            hd_print("Channels not loaded");
        }

        $items = array();

        foreach ($this->plugin->tv->get_groups() as $group) {
            $media_url_str = $group->is_favorite_channels() ?
                Tv_Favorites_Screen::get_media_url_str() :
                Tv_Channel_List_Screen::get_media_url_str($group->get_id());

            $items[] = array(
                PluginRegularFolderItem::media_url => $media_url_str,
                PluginRegularFolderItem::caption => $group->get_title(),
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $group->get_icon_url(),
                    ViewItemParams::item_detailed_icon_path => $group->get_icon_url()
                )
            );
        }

        $this->plugin->tv->add_special_groups($items);

        // hd_print("Loaded items " . count($items));
        return $items;
    }

    /**
     * @param MediaURL $media_url
     * @return Archive|null
     */
    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->tv->get_archive($media_url);
    }
}

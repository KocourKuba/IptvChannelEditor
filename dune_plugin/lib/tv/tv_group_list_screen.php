<?php

require_once 'lib/abstract_preloaded_regular_screen.php';

class TvGroupListScreen extends AbstractPreloadedRegularScreen
{
    const ID = 'tv_group_list';

    ///////////////////////////////////////////////////////////////////////

    protected $plugin;

    ///////////////////////////////////////////////////////////////////////

    public function __construct(DefaultDunePlugin $plugin)
    {
        $this->plugin = $plugin;

        parent::__construct(self::ID, $this->plugin->config->GET_TV_GROUP_LIST_FOLDER_VIEWS());
    }

    ///////////////////////////////////////////////////////////////////////

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER => ActionFactory::open_folder(),
            GUI_EVENT_KEY_PLAY => ActionFactory::tv_play(),
            GUI_EVENT_KEY_B_GREEN => ActionFactory::open_folder(StarnetSetupScreen::get_media_url_str(), 'Настройки плагина'),
        );
    }

    ///////////////////////////////////////////////////////////////////////

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
                TvFavoritesScreen::get_media_url_str() :
                TvChannelListScreen::get_media_url_str($group->get_id());

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

    public function get_archive(MediaURL $media_url)
    {
        return $this->plugin->tv->get_archive($media_url);
    }
}

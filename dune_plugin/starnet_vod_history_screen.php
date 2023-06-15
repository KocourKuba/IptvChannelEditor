<?php
require_once 'lib/abstract_preloaded_regular_screen.php';
require_once 'lib/vod/vod.php';

class Starnet_Vod_History_Screen extends Abstract_Preloaded_Regular_Screen implements User_Input_Handler
{
    const ID = 'vod_history';

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
        parent::__construct(self::ID, $plugin, $plugin->vod->get_vod_list_folder_views());

        if ($plugin->config->get_feature(Plugin_Constants::VOD_SUPPORTED)) {
            $plugin->create_screen($this);
        }
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     */
    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER    => $this->plugin->vod->is_movie_page_supported() ? Action_Factory::open_folder() : Action_Factory::vod_play(),
            GUI_EVENT_KEY_PLAY     => Action_Factory::vod_play(),
            GUI_EVENT_KEY_B_GREEN  => User_Input_Handler_Registry::create_action($this, ACTION_ITEM_DELETE, TR::t('delete')),
            GUI_EVENT_KEY_C_YELLOW => User_Input_Handler_Registry::create_action($this, ACTION_ITEMS_CLEAR, TR::t('clear_history')),
            GUI_EVENT_KEY_D_BLUE   => User_Input_Handler_Registry::create_action($this, ACTION_ADD_FAV, TR::t('add_to_favorite')),
        );
    }

    /**
     * @return string
     */
    public function get_handler_id()
    {
        return self::ID;
    }

    /**
     * @param $user_input
     * @param $plugin_cookies
     * @return array|null
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //dump_input_handler(__METHOD__, $user_input);

        if (!isset($user_input->selected_media_url)) {
            return null;
        }

        $media_url = MediaURL::decode($user_input->selected_media_url);
        $movie_id = $media_url->movie_id;

        switch ($user_input->control_id)
		{
			case ACTION_ITEM_DELETE:
				$media_url = MediaURL::decode($user_input->selected_media_url);
                //hd_print(__METHOD__ . ": Delete movie_id: $media_url->movie_id");
                $this->plugin->vod->remove_movie_from_history($media_url->movie_id, $plugin_cookies);
				$parent_media_url = MediaURL::decode($user_input->parent_media_url);
				$sel_ndx = $user_input->sel_ndx + 1;
				if ($sel_ndx < 0)
					$sel_ndx = 0;
				$range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
				return Action_Factory::update_regular_folder($range, true, $sel_ndx);

            case ACTION_ITEMS_CLEAR:
                $this->plugin->vod->set_history_items(array(), $plugin_cookies);
                HD::erase_items(Starnet_Vod::VOD_HISTORY_ITEMS . "_" . $this->plugin->config->get_vod_template_name($plugin_cookies));
                $parent_media_url = MediaURL::decode($user_input->parent_media_url);
                $range = $this->get_folder_range($parent_media_url, 0, $plugin_cookies);
                return Action_Factory::update_regular_folder($range, true);

            case ACTION_ADD_FAV:
				$is_favorite = $this->plugin->vod->is_favorite_movie_id($movie_id);
				$opt_type = $is_favorite ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
				$message = $is_favorite ? TR::t('deleted_from_favorite') : TR::t('added_to_favorite');
				$this->plugin->vod->change_vod_favorites($opt_type, $movie_id, $plugin_cookies);
				return Action_Factory::show_title_dialog($message);
		}

        return null;
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function get_all_folder_items(MediaURL $media_url, &$plugin_cookies)
    {
        //hd_print("Starnet_Vod_History_Screen: get_all_folder_items");

        $this->plugin->vod->ensure_history_loaded($plugin_cookies);
        $history_items = $this->plugin->vod->get_history_movies();

        $items = array();
        foreach ($history_items as $movie_id => $movie_infos) {
            if (empty($movie_infos)) continue;

            $this->plugin->vod->ensure_movie_loaded($movie_id, $plugin_cookies);
            $short_movie = $this->plugin->vod->get_cached_short_movie($movie_id);

            if (is_null($short_movie)) {
                $caption = TR::t('vod_screen_no_film_info');
                $poster_url = "missing://";
            } else {
                $last_viewed = 0;
                foreach ($movie_infos as $info) {
                    if (isset($info[Movie::WATCHED_DATE])) {
                        $last_viewed = max($last_viewed, $info[Movie::WATCHED_DATE]);
                        //hd_print("get_all_folder_items: info $key => {$info[Movie::WATCHED_DATE]}");
                    }
                }

                if ($last_viewed !== 0)
                    $caption = TR::t('vod_screen_last_viewed__2', $short_movie->name, format_datetime("d.m.Y H:i", $last_viewed));
                else
                    $caption = $short_movie->name;

                $poster_url = $short_movie->poster_url;
            }

            $items[] = array
            (
                PluginRegularFolderItem::media_url => Starnet_Vod_Movie_Screen::get_media_url_str($movie_id),
                PluginRegularFolderItem::caption => $caption,
                PluginRegularFolderItem::view_item_params => array
                (
                    ViewItemParams::icon_path => $poster_url,
                )
            );
        }

        return $items;
    }
}

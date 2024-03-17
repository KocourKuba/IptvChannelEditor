<?php
require_once 'lib/ext_epg_program.php';

require_once 'lib/epfs/abstract_rows_screen.php';
require_once 'lib/epfs/rows_factory.php';
require_once 'lib/epfs/gcomps_factory.php';
require_once 'lib/epfs/gcomp_geom.php';
require_once 'lib/playback_points.php';

class Starnet_Tv_Rows_Screen extends Abstract_Rows_Screen implements User_Input_Handler
{
    const ID = 'rows_epf';

    ///////////////////////////////////////////////////////////////////////////

    private $remove_playback_point;
    private $removed_playback_point;
    private $clear_playback_points = false;

    public $need_update_epf_mapping_flag = false;

    ///////////////////////////////////////////////////////////////////////////

    /**
     * @param $pane
     * @param $rows_before
     * @param $rows_after
     * @param $min_row_index_for_y2
     * @return void
     */
    public function add_rows_to_pane(&$pane, $rows_before = null, $rows_after = null, $min_row_index_for_y2 = null)
    {
        if (is_array($rows_before))
            $pane[PluginRowsPane::rows] = array_merge($rows_before, $pane[PluginRowsPane::rows]);

        if (is_array($rows_after))
            $pane[PluginRowsPane::rows] = array_merge($pane[PluginRowsPane::rows], $rows_after);

        if (!is_null($min_row_index_for_y2))
            $pane[PluginRowsPane::min_row_index_for_y2] = $min_row_index_for_y2;
    }

    public function get_parent_media_url($parent_sel_state)
    {
        foreach (explode("\n", $parent_sel_state) as $line) {
            if (strpos($line, 'channel_id')) {
                return MediaURL::decode($line);
            }
        }

        return null;
    }

    /**
     * @param $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    protected function do_get_info_children($media_url, $plugin_cookies)
    {
        hd_debug_print(null, true);

        $group_id = isset($media_url->group_id) ? $media_url->group_id : null;
        $channel_id = isset($media_url->channel_id) ? $media_url->channel_id : null;

        if (is_null($channel_id) || empty($group_id))
            return null;

        $channel = $this->plugin->tv->get_channel($channel_id);
        if (is_null($channel)) {
            hd_debug_print("Unknown channel $channel_id");
            return null;
        }

        $title_num = 1;
        $defs = array();

        ///////////// Channel number /////////////////

        $number = $channel->get_number();
        $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(130, 50, 690, 520),
            null,
            $number,
            1,
            PaneParams::ch_num_font_color,
            PaneParams::ch_num_font_size,
            'ch_number'
        );

        ///////////// Channel title /////////////////

        $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width + 200, PaneParams::prog_item_height),
            null,
            $channel->get_title(),
            1,
            PaneParams::ch_title_font_color,
            PaneParams::ch_title_font_size,
            'ch_title'
        );
        $y = PaneParams::prog_item_height;

        ///////////// start_time, end_time, genre, country, person /////////////////

        if (is_null($epg_data = $this->plugin->get_program_info($channel_id, -1, $plugin_cookies))) {

            hd_debug_print("no epg data");
            $channel_desc = $channel->get_desc();
            if (!empty($channel_desc)) {
                $defs[] = GComps_Factory::label(
                    GComp_Geom::place_top_left(PaneParams::info_width, -1, 0, $y),
                    null,
                    $channel_desc,
                    13 - $title_num,
                    PaneParams::prog_item_font_color,
                    PaneParams::prog_item_font_size,
                    'ch_desc',
                    array('line_spacing' => 6)
                );
            }
        } else {

            $program = (object)array();
            $program->time = sprintf("%s - %s",
                gmdate('H:i', $epg_data[PluginTvEpgProgram::start_tm_sec] + get_local_time_zone_offset()),
                gmdate('H:i', $epg_data[PluginTvEpgProgram::end_tm_sec] +  get_local_time_zone_offset())
            );
            //$program->year = preg_match('/\s+\((\d{4,4})\)$/', $epg_data[Ext_Epg_Program::main_category], $matches) ? $matches[1] : '';
            //$program->age = preg_match('/\s+\((\d{1,2}\+)\)$/', $epg_data[Ext_Epg_Program::main_category], $matches) ? $matches[1] : '';

            $title = $epg_data[PluginTvEpgProgram::name];
            $desc = (!empty($epg_data[Ext_Epg_Program::sub_title]) ? $epg_data[Ext_Epg_Program::sub_title] . "\n" : '') . $epg_data[PluginTvEpgProgram::description];
            $fanart_url = '';

            // duration
            $defs[] = GComps_Factory::label(
                GComp_Geom::place_top_left(PaneParams::info_width, PaneParams::prog_item_height, 0, $y),
                null,
                $program->time,
                1,
                PaneParams::prog_item_font_color,
                PaneParams::prog_item_font_size);
            $y += PaneParams::prog_item_height;

            ///////////// Program title ////////////////

            if (!empty($title)) {
                $lines = array_slice(explode("\n",
                    iconv('Windows-1251', 'UTF-8',
                        wordwrap(iconv('UTF-8', 'Windows-1251',
                            trim(preg_replace('/([!?])\.+\s*$/Uu', '$1', $title))),
                            40, "\n", true)
                    )),
                    0, 2);

                $prog_title = implode("\n", $lines);

                if (strlen($prog_title) < strlen($title))
                    $prog_title = $title;

                $lines = min(2, count($lines));
                $defs[] = GComps_Factory::label(
                    GComp_Geom::place_top_left(PaneParams::info_width + 100, PaneParams::prog_item_height, 0, $y + ($lines > 1 ? 20 : 0)),
                    null,
                    $prog_title,
                    2,
                    PaneParams::prog_title_font_color,
                    PaneParams::prog_title_font_size,
                    'prog_title',
                    array('line_spacing' => 5)
                );
                $y += (PaneParams::prog_item_height - 20) * $lines + ($lines > 1 ? 10 : 0);
                $title_num += $lines > 1 ? 1 : 0;
            } else {
                $title_num--;
            }

            ///////////// Description ////////////////

            if (!empty($desc)) {
                $defs[] = GComps_Factory::label(
                    GComp_Geom::place_top_left(PaneParams::info_width, -1, 0, $y + 5),
                    null,
                    $desc,
                    10 - $title_num,
                    PaneParams::prog_item_font_color,
                    PaneParams::prog_item_font_size,
                    'prog_desc',
                    array('line_spacing' => 5)
                );
            }
        }

        // separator line
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(510, 4, 0, 590), null, PaneParams::separator_line_color);

        $in_fav = $this->plugin->get_favorites()->in_order($channel_id);
        $dy_icon = 530;
        $dy_txt = $dy_icon - 4;
        $dx = 15;
        if ($group_id !== FAVORITES_GROUP_ID) {
            hd_debug_print("newUI: $group_id");
            // blue button image (D)
            $defs[] = GComps_Factory::get_image_def(
                GComp_Geom::place_top_left(PaneParams::fav_btn_width, PaneParams::fav_btn_height, $dx, $dy_icon),
                null,
                PaneParams::fav_button_blue);

            $dx += 55;
            $defs[] = GComps_Factory::label(
                GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                TR::t('add_delete_favorite'),
                1,
                PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );
        } else {
            hd_debug_print("newUI: " . FAVORITES_GROUP_ID);
            $order = $this->plugin->get_favorites()->get_order();
            $is_first_channel = ($channel_id === reset($order));

            // green button image (B) 52x50
            $defs[] = GComps_Factory::get_image_def(
                GComp_Geom::place_top_left(PaneParams::fav_btn_width, PaneParams::fav_btn_height, $dx, $dy_icon),
                null,
                PaneParams::fav_button_green,
                false,
                true,
                null,
                null,
                null,
                $is_first_channel ? 99 : 255);

            $dx += 55;
            // green button text
            $defs[] = GComps_Factory::label(
                GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                TR::t('left'),
                1,
                $is_first_channel ? PaneParams::fav_btn_disabled_font_color : PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );

            $is_last_channel = ($channel_id === end($order));
            $dx += 105;
            // yellow button image (C)
            $defs[] = GComps_Factory::get_image_def(
                GComp_Geom::place_top_left(PaneParams::fav_btn_width, PaneParams::fav_btn_height, $dx, $dy_icon),
                null,
                PaneParams::fav_button_yellow,
                1,
                false,
                null,
                null,
                null,
                $is_last_channel ? 99 : 255
            );

            $dx += 55;
            // yellow button text
            $defs[] = GComps_Factory::label(
                GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                TR::t('right'),
                1,
                $is_last_channel ? PaneParams::fav_btn_disabled_font_color : PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );

            $dx += 105;
            // blue button image (D)
            $defs[] = GComps_Factory::get_image_def(
                GComp_Geom::place_top_left(PaneParams::fav_btn_width, PaneParams::fav_btn_height, $dx, $dy_icon),
                null,
                PaneParams::fav_button_blue);

            $dx += 55;
            // blue button text
            $defs[] = GComps_Factory::label(
                GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                $in_fav ? TR::t('delete') : TR::t('add'),
                1,
                PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );
        }

        ///////////// Enclosing panel ////////////////

        $pane_def = GComps_Factory::get_panel_def(
            'info_pane',
            GComp_Geom::place_top_left(PaneParams::pane_width, PaneParams::pane_height),
            null,
            $defs,
            GCOMP_OPT_PREPAINT
        );
        GComps_Factory::add_extra_var($pane_def, 'info_inf_dimmed', null, array('alpha' => 64));

        return array(
            'defs' => array($pane_def),
            'fanart_url' => empty($fanart_url) ? '' : $fanart_url,
        );
    }

    /**
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_folder_view_for_epf(&$plugin_cookies)
    {
        hd_debug_print(null, true);

        if ($this->plugin->tv->load_channels() === -1) {
            hd_debug_print("Channels not loaded!");
        }

        return $this->get_folder_view(MediaURL::decode(self::ID), $plugin_cookies);
    }

    public function get_folder_range(MediaURL $media_url, $from_ndx, &$plugin_cookies)
    {
        return null;
    }

    public function get_next_folder_view(MediaURL $media_url, &$plugin_cookies)
    {
        return null;
    }

    /**
     * @inheritDoc
     */
    public function get_rows_pane(MediaURL $media_url, $plugin_cookies)
    {
        hd_debug_print(null, true);
        $rows = array();

        $channels_rows = $this->get_regular_rows();
        if (is_null($channels_rows)) {
            hd_debug_print("no channels rows");
            return null;
        }

        $history_rows = $this->get_history_rows($plugin_cookies);
        if (!is_null($history_rows)) {
            $rows = array_merge($rows, $history_rows);
            hd_debug_print("added history: " . count($history_rows) . " rows", true);
        }

        $favorites_rows = $this->get_favorites_rows();
        if (!is_null($favorites_rows)) {
            hd_debug_print("added favorites: " . count($favorites_rows) . " rows", true);
            $rows = array_merge($rows, $favorites_rows);
        }

        $all_channels_rows = $this->get_all_channels_row();
        if (!is_null($all_channels_rows)) {
            $rows = array_merge($rows, $all_channels_rows);
            hd_debug_print("added all channels: " . count($all_channels_rows) . " rows", true);
        }

        $rows = array_merge($rows, $channels_rows);
        hd_debug_print("added channels: " . count($channels_rows) . " rows", true);

        $pane = Rows_Factory::pane(
            $rows,
            Rows_Factory::focus(GCOMP_FOCUS_DEFAULT_CUT_IMAGE, GCOMP_FOCUS_DEFAULT_RECT),
            null, true, true, -1, null, null,
            1.0, 0.0, -0.5, 250);

        Rows_Factory::pane_set_geometry(
            $pane,
            PaneParams::width,
            PaneParams::height,
            PaneParams::dx,
            PaneParams::dy,
            PaneParams::info_height,
            empty($history_rows) ? 1 : 2,
            PaneParams::width - PaneParams::info_dx,
            PaneParams::info_height - PaneParams::info_dy,
            PaneParams::info_dx, PaneParams::info_dy,
            PaneParams::vod_width, PaneParams::vod_height
        );

        $square_icons = $this->plugin->get_bool_parameter(PARAM_SQUARE_ICONS);
        $icon_width = $square_icons ? RowsItemsParams::icon_width_sq : RowsItemsParams::icon_width;
        $icon_prop = $icon_width / RowsItemsParams::icon_height;

        $def_params = Rows_Factory::variable_params(
            RowsItemsParams::width,
            RowsItemsParams::height,
            0,
            $icon_width,
            RowsItemsParams::icon_height,
            5,
            RowsItemsParams::caption_dy,
            RowsItemsParams::def_caption_color,
            RowsItemsParams::caption_font_size
        );

        $sel_icon_width = $icon_width + 12;
        $sel_icon_height = round(($icon_width + 12) / $icon_prop);
        $sel_params = Rows_Factory::variable_params(
            RowsItemsParams::width,
            RowsItemsParams::height,
            5,
            $sel_icon_width,
            $sel_icon_height,
            0,
            RowsItemsParams::caption_dy + 10,
            RowsItemsParams::sel_caption_color,
            RowsItemsParams::caption_font_size
        );

        $width = round((RowsItemsParams::width * PaneParams::max_items_in_row - PaneParams::group_list_width) / PaneParams::max_items_in_row);
        $inactive_icon_width = round(($icon_width * PaneParams::max_items_in_row - PaneParams::group_list_width) / PaneParams::max_items_in_row)
            + round((RowsItemsParams::width - $icon_width) / $icon_prop);
        $inactive_icon_height = round($inactive_icon_width / $icon_prop) - 10;

        $inactive_params = Rows_Factory::variable_params(
            $width,
            round($width / RowsItemsParams::width * RowsItemsParams::height), 0,
            $inactive_icon_width,
            $inactive_icon_height,
            0,
            RowsItemsParams::caption_dy,
            RowsItemsParams::inactive_caption_color,
            RowsItemsParams::caption_font_size
        );

        $params = Rows_Factory::item_params(
            $def_params,
            $sel_params,
            $inactive_params,
            get_image_path($square_icons ? RowsItemsParams::icon_sq_loading_url : RowsItemsParams::icon_loading_url),
            get_image_path($square_icons ? RowsItemsParams::icon_sq_loading_failed_url : RowsItemsParams::icon_loading_failed_url),
            RowsItemsParams::caption_max_num_lines,
            RowsItemsParams::caption_line_spacing,
            Rows_Factory::margins(6, 2, 2, 2)
        );

        Rows_Factory::set_item_params_template($pane, 'common', $params);

        return $pane;
    }

    public function get_action_map(MediaURL $media_url, &$plugin_cookies)
    {
        return array(
            GUI_EVENT_KEY_ENTER               => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER),
            GUI_EVENT_KEY_B_GREEN             => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_MOVE_UP),
            GUI_EVENT_KEY_C_YELLOW            => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_MOVE_DOWN),
            GUI_EVENT_KEY_D_BLUE              => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_ADD),
            GUI_EVENT_KEY_CLEAR               => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_REMOVE),
            GUI_EVENT_KEY_POPUP_MENU          => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_PLUGIN_ROWS_INFO_UPDATE => User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLUGIN_ROWS_INFO_UPDATE),
        );
    }

    /**
     * @inheritDoc
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        hd_debug_print(null, true);
        dump_input_handler($user_input);

        // handler_id => rows_epf_handler
        // control_id => move_up
        // play_mode => none
        // parent_media_url => iedem.tv
        // selected_header => 0
        // selected_row_id => {"row_ndx":0,"row_id":"{\"group_id\":\"##favorites##\"}"}
        // selected_item_id => {"group_id":"##favorites##","channel_id":"205","fav_idx":"2\/3"}
        // selected_title_ndx => 1
        // parent_sel_state => 1\n
        // 0\n
        // H1\n
        // {"row_ndx":0,"row_id":"{\"group_id\":\"##favorites##\"}"}
        // {"group_id":"##favorites##","channel_id":"2024","fav_idx":"2\/3"}
        // pri:1\n
        // {"group_id":"##favorites##","channel_id":"2024","fav_idx":"2\/3"}
        // 1\n
        // 1\n

        if (isset($user_input->item_id)) {
            $media_url_str = $user_input->item_id;
            $media_url = MediaURL::decode($media_url_str);
        } else if ($user_input->control_id === ACTION_REFRESH_SCREEN) {
            $media_url = '';
            $media_url_str = '';
        } else {
            $media_url = $this->get_parent_media_url($user_input->parent_sel_state);
            $media_url_str = '';
            if (is_null($media_url))
                return null;
        }

        $this->set_cur_sel_state_str(isset($user_input->parent_sel_state) ? $user_input->parent_sel_state : null);

        $control_id = $user_input->control_id;

        switch ($control_id) {
            case GUI_EVENT_TIMER:
                // rising after playback end + 100 ms
                $this->plugin->get_playback_points()->update_point(null);
                return User_Input_Handler_Registry::create_action($this, ACTION_REFRESH_SCREEN);

            case GUI_EVENT_KEY_ENTER:
                $tv_play_action = Action_Factory::tv_play($media_url);

                if (isset($user_input->action_origin)) {
                    Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                    return Action_Factory::close_and_run(Starnet_Epfs_Handler::invalidate_folders(null, $tv_play_action));
                }

                $new_actions = array_merge($this->get_action_map($media_url, $plugin_cookies),
                    array(GUI_EVENT_TIMER => User_Input_Handler_Registry::create_action($this, GUI_EVENT_TIMER)));

                return Action_Factory::change_behaviour($new_actions, 100, $tv_play_action);

            case GUI_EVENT_PLUGIN_ROWS_INFO_UPDATE:
                if (!isset($user_input->item_id, $user_input->folder_key))
                    return null;

                $info_children = $this->do_get_info_children(MediaURL::decode($media_url_str), $plugin_cookies);

                return Action_Factory::update_rows_info(
                    $user_input->folder_key,
                    $user_input->item_id,
                    $info_children['defs'],
                    empty($info_children['fanart_url']) ? get_image_path(PaneParams::vod_bg_url) : $info_children['fanart_url'],
                    get_image_path(PaneParams::vod_bg_url),
                    get_image_path(PaneParams::vod_mask_url),
                    array("plugin_tv://" . get_plugin_name() . "/$user_input->item_id")
                );

            case GUI_EVENT_KEY_POPUP_MENU:
                $menu_items = array();
                if (isset($user_input->selected_item_id)) {
                    if ($media_url->group_id === HISTORY_GROUP_ID) {
                        $this->create_menu_item($menu_items, TR::t('delete'), get_image_path('remove.png'), PLUGIN_FAVORITES_OP_REMOVE);
                    } else {
                        // popup menu for channel
                        $menu_items = $this->channel_menu($media_url);
                    }
                } else {
                    // common popup menu
                    if ($media_url->group_id === HISTORY_GROUP_ID) {
                        $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_CLEAR, TR::t('clear_history'), "brush.png");
                    } else if ($media_url->group_id === FAVORITES_GROUP_ID) {
                        $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEMS_CLEAR, TR::t('clear_favorites'), "brush.png");
                    }

                    $cache_engine = $this->plugin->get_parameter(PARAM_EPG_CACHE_ENGINE, ENGINE_JSON);
                    if ($cache_engine === ENGINE_XMLTV) {
                        $menu_items = $this->plugin->epg_source_menu($this);
                    }
                    $menu_items[] = $this->plugin->create_menu_item($this, ACTION_TOGGLE_ICONS_TYPE, TR::t('tv_screen_toggle_icons_aspect'), "image.png");

                    $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);
                    $menu_items[] = $this->plugin->create_menu_item($this, ACTION_REFRESH_SCREEN, TR::t('refresh'), "refresh.png");
                }

                return Action_Factory::show_popup_menu($menu_items);

            case ACTION_ZOOM_POPUP_MENU:
                $menu_items = array();
                $zoom_data = $this->plugin->get_channel_zoom($media_url->channel_id);
                foreach (DuneVideoZoomPresets::$zoom_ops as $idx => $zoom_item) {
                    $menu_items[] = $this->plugin->create_menu_item($this,
                        ACTION_ZOOM_APPLY,
                        TR::t($zoom_item),
                        (strcmp($idx, $zoom_data) !== 0 ? null : "check.png"),
                        array(ACTION_ZOOM_SELECT => (string)$idx)
                    );
                }

                return Action_Factory::show_popup_menu($menu_items);

            case ACTION_EPG_SOURCE_SELECTED:
                if (!isset($user_input->list_idx)) break;

                $this->plugin->set_active_xmltv_source_key($user_input->list_idx);
                $this->plugin->tv->reload_channels();
                $this->plugin->set_need_update_epfs();

                return $this->plugin->invalidate_epfs_folders($plugin_cookies);

            case PLUGIN_FAVORITES_OP_ADD:
            case PLUGIN_FAVORITES_OP_REMOVE:
                if (!isset($media_url->group_id)) break;

                if ($media_url->group_id === HISTORY_GROUP_ID && $control_id === PLUGIN_FAVORITES_OP_REMOVE) {
                    $this->plugin->get_playback_points()->erase_point($media_url->channel_id);
                    $this->plugin->set_need_update_epfs();
                    return $this->plugin->invalidate_epfs_folders($plugin_cookies);
                }

                if ($control_id === PLUGIN_FAVORITES_OP_ADD) {
                    $control_id = $this->plugin->get_favorites()->in_order($media_url->channel_id) ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                }
                $action = $this->plugin->change_tv_favorites($control_id, $media_url->channel_id);
                $this->plugin->save_favorites();
                return $this->plugin->update_invalidate_epfs_folders($plugin_cookies, $action);

            case PLUGIN_FAVORITES_OP_MOVE_UP:
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                if (!isset($media_url->group_id)) break;

                if ($media_url->group_id === FAVORITES_GROUP_ID) {
                    $action = $this->plugin->change_tv_favorites($control_id, $media_url->channel_id);
                    $this->plugin->save_favorites();
                    $user_input->parent_sel_state = null;
                    return $this->plugin->update_invalidate_epfs_folders($plugin_cookies, $action);
                }
                break;

            case ACTION_ITEMS_CLEAR:
                if (!isset($media_url->group_id)) break;

                if ($media_url->group_id === HISTORY_GROUP_ID) {
                    $this->clear_playback_points = true;
                    $this->plugin->get_playback_points()->clear_points();
                    return User_Input_Handler_Registry::create_action($this, ACTION_REFRESH_SCREEN);
                }

                if ($media_url->group_id === FAVORITES_GROUP_ID) {
                    $action = $this->plugin->change_tv_favorites(ACTION_ITEMS_CLEAR, null);
                    $this->plugin->save_favorites();
                    return $this->plugin->update_invalidate_epfs_folders($plugin_cookies, $action);
                }
                break;

            case ACTION_ZOOM_APPLY:
                if (isset($user_input->{ACTION_ZOOM_SELECT})) {
                    $zoom_select = $user_input->{ACTION_ZOOM_SELECT};
                    $this->plugin->set_channel_zoom($media_url->channel_id, ($zoom_select !== DuneVideoZoomPresets::not_set) ? $zoom_select : null);
                }
                break;

            case ACTION_EXTERNAL_PLAYER:
            case ACTION_INTERNAL_PLAYER:
                $this->plugin->set_channel_for_ext_player($media_url->channel_id, $user_input->control_id === ACTION_EXTERNAL_PLAYER);
                break;

            case ACTION_TOGGLE_ICONS_TYPE:
                $new = $this->plugin->toggle_parameter(PARAM_SQUARE_ICONS, SetupControlSwitchDefs::switch_off);
                hd_debug_print("new value $new");
                return User_Input_Handler_Registry::create_action($this, ACTION_REFRESH_SCREEN);

            case ACTION_REFRESH_SCREEN:
                $this->plugin->set_need_update_epfs();
                return Action_Factory::invalidate_all_folders($plugin_cookies);
        }

        return null;
    }

    /**
     * @param $menu_items array
     * @param $caption string
     * @param $icon string
     * @param $action_id string
     * @param $add_params array|null
     * @return void
     * @noinspection PhpSameParameterValueInspection
     */
    private function create_menu_item(&$menu_items, $caption, $icon, $action_id, $add_params = null)
    {
        $menu_items[] = array(
            GuiMenuItemDef::caption => $caption,
            GuiMenuItemDef::icon_url => $icon,
            GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, $action_id, null, $add_params),
        );
    }

    /**
     * @param MediaURL $media_url
     * @return array
     */
    protected function channel_menu(MediaURL $media_url)
    {
        hd_debug_print(null, true);

        if ($media_url->group_id === HISTORY_GROUP_ID) {
            $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ITEM_REMOVE, TR::t('delete'), "remove.png");
        } else {
            $channel_id = $media_url->channel_id;
            //hd_debug_print("Selected channel id: $channel_id");

            $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);

            if (!is_limited_apk()) {
                $is_external = $this->plugin->is_channel_for_ext_player($channel_id);
                $menu_items[] = $this->plugin->create_menu_item($this,
                    ACTION_EXTERNAL_PLAYER,
                    TR::t('tv_screen_external_player'),
                    ($is_external ? "play.png" : null)
                );

                $menu_items[] = $this->plugin->create_menu_item($this,
                    ACTION_INTERNAL_PLAYER,
                    TR::t('tv_screen_internal_player'),
                    ($is_external ? null : "play.png")
                );

                $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);
            }

            if ($this->plugin->get_bool_parameter(PARAM_PER_CHANNELS_ZOOM)) {
                $menu_items[] = $this->plugin->create_menu_item($this, ACTION_ZOOM_POPUP_MENU, TR::t('video_aspect_ratio'), "aspect.png");
            }

            if (is_limited_apk()) {
                $is_in_favorites = $this->plugin->get_favorites()->in_order($channel_id);

                $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);
                if ($media_url->group_id === FAVORITES_GROUP_ID) {
                    $this->create_menu_item($menu_items, TR::t('left'), PaneParams::fav_button_green, PLUGIN_FAVORITES_OP_MOVE_UP);
                    $this->create_menu_item($menu_items, TR::t('right'), PaneParams::fav_button_yellow, PLUGIN_FAVORITES_OP_MOVE_DOWN);
                }
                $this->create_menu_item($menu_items,
                    $is_in_favorites ? TR::t('delete') : TR::t('add'),
                    PaneParams::fav_button_blue,
                    $is_in_favorites ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD);
            }
        }

        $menu_items[] = $this->plugin->create_menu_item($this, GuiMenuItemDef::is_separator);
        $menu_items[] = $this->plugin->create_menu_item($this, ACTION_REFRESH_SCREEN, TR::t('refresh'), "refresh.png");

        return $menu_items;
    }

    ////////////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return array
     */
    private function get_history_rows($plugin_cookies)
    {
        hd_debug_print(null, true);
        if (!$this->plugin->get_bool_parameter(PARAM_SHOW_HISTORY)) {
            hd_debug_print("History group disabled");
            return null;
        }

        if ($this->clear_playback_points) {
            $this->clear_playback_points = false;
            return null;
        }

        // Fill view history data
        $now = time();
        $watched = array();
        foreach ($this->plugin->get_playback_points()->get_all() as $channel_id => $channel_ts) {
            if (is_null($channel = $this->plugin->tv->get_channel($channel_id))) continue;

            $prog_info = $this->plugin->get_program_info($channel_id, $channel_ts, $plugin_cookies);
            $progress = 0;

            if (is_null($prog_info)) {
                $title = $channel->get_title();
            } else {
                // program epg available
                $title = $prog_info[PluginTvEpgProgram::name];
                if ($channel_ts > 0) {
                    $start_tm = $prog_info[PluginTvEpgProgram::start_tm_sec];
                    $epg_len = $prog_info[PluginTvEpgProgram::end_tm_sec] - $start_tm;
                    if ($channel_ts >= $now - $channel->get_archive_past_sec() - 60) {
                        $progress = max(0.01, min(1.0, round(($channel_ts - $start_tm) / $epg_len, 2)));
                    }
                }
            }

            $watched[(string)$channel_id] = array(
                'channel_id' => $channel_id,
                'archive_tm' => $channel_ts,
                'view_progress' => $progress,
                'program_title' => $title,
            );
        }

        // fill view history row items
        $items = array();
        foreach ($watched as $item) {
            if (!is_null($channel = $this->plugin->tv->get_channel($item['channel_id']))) {
                $id = json_encode(array('group_id' => HISTORY_GROUP_ID, 'channel_id' => $item['channel_id'], 'archive_tm' => $item['archive_tm']));
                if (isset($this->removed_playback_point))
                    if ($this->removed_playback_point === $id) {
                        $this->removed_playback_point = null;
                        $this->plugin->get_playback_points()->erase_point($item['channel_id']);
                        continue;
                    }

                $stickers = null;

                if ($item['view_progress'] > 0) {
                    // item size 229x142
                    if (!empty($item['program_icon_url'])) {
                        // add small channel logo
                        $rect = Rows_Factory::r(129, 0, 100, 64);
                        $stickers[] = Rows_Factory::add_regular_sticker_rect(RowsItemsParams::fav_sticker_logo_bg_color, $rect);
                        $stickers[] = Rows_Factory::add_regular_sticker_image($channel->get_icon_url(), $rect);
                    }

                    // add progress indicator
                    $stickers[] = Rows_Factory::add_regular_sticker_rect(
                        RowsItemsParams::view_total_color,
                        Rows_Factory::r(0,
                            RowsItemsParams::fav_progress_dy,
                            RowsItemsParams::view_progress_width,
                            RowsItemsParams::view_progress_height)); // total

                    $stickers[] = Rows_Factory::add_regular_sticker_rect(
                        RowsItemsParams::view_viewed_color,
                        Rows_Factory::r(0,
                            RowsItemsParams::fav_progress_dy,
                            round(RowsItemsParams::view_progress_width * $item['view_progress']),
                            RowsItemsParams::view_progress_height)); // viewed
                }

                $items[] = Rows_Factory::add_regular_item(
                    $id,
                    $channel->get_icon_url(),
                    $item['program_title'],
                    $stickers);
            }
        }

        if (empty($items)) {
            return null;
        }

        // create view history group
        return $this->create_rows($items,
            json_encode(array('group_id' => HISTORY_GROUP_ID)),
            TR::t('tv_screen_continue'),
            TR::t('tv_screen_continue_view'),
            null,
            TitleRowsParams::history_caption_color
        );
    }

    /**
     * @return array|null
     */
    private function get_favorites_rows()
    {
        hd_debug_print(null, true);

        $group = $this->plugin->tv->get_special_group(FAVORITES_GROUP_ID);
        if (is_null($group)) {
            hd_debug_print("Favorites group not found");
            return null;
        }

        if ($group->is_disabled()) {
            hd_debug_print("Favorites group disabled");
            return null;
        }

        foreach ($this->plugin->get_favorites() as $channel_id) {
            $channel = $this->plugin->tv->get_channel($channel_id);
            if (is_null($channel) || $channel->is_disabled()) continue;

            $items[] = Rows_Factory::add_regular_item(
                json_encode(array('group_id' => FAVORITES_GROUP_ID, 'channel_id' => $channel_id)),
                $channel->get_icon_url(),
                $channel->get_title()
            );
        }

        if (empty($items)) {
            return null;
        }

        return $this->create_rows($items,
            json_encode(array('group_id' => FAVORITES_GROUP_ID)),
            $group->get_title(),
            $group->get_title(),
            User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER),
            TitleRowsParams::fav_caption_color
        );
    }

    /**
     * @return array|null
     */
    private function get_all_channels_row()
    {
        hd_debug_print(null, true);

        $all_group = $this->plugin->tv->get_special_group(ALL_CHANNEL_GROUP_ID);
        if (is_null($all_group)) {
            hd_debug_print("All channels group not found");
            return null;
        }

        if ($all_group->is_disabled()) {
            hd_debug_print("All channels group disabled");
            return null;
        }

        $items = array();
        $square_icons =  $this->plugin->get_parameter(PARAM_SQUARE_ICONS);
        $row_item_width = $square_icons ? RowsItemsParams::width_sq : RowsItemsParams::width;

        $fav_stickers[] = Rows_Factory::add_regular_sticker_rect(
            RowsItemsParams::fav_sticker_bg_color,
            Rows_Factory::r(
                $row_item_width - RowsItemsParams::fav_sticker_bg_width - 21,
                0,
                RowsItemsParams::fav_sticker_bg_width,
                RowsItemsParams::fav_sticker_bg_width));

        $fav_stickers[] = Rows_Factory::add_regular_sticker_image(
            get_image_path(RowsItemsParams::fav_sticker_icon_url),
            Rows_Factory::r(
                $row_item_width - RowsItemsParams::fav_sticker_icon_width - 23,
                2,
                RowsItemsParams::fav_sticker_icon_width,
                RowsItemsParams::fav_sticker_icon_height));

        $channels_order = new Hashed_Array();
        foreach($this->plugin->tv->get_groups()->get_order() as $group_id) {
            $group = $this->plugin->tv->get_group($group_id);
            if (is_null($group)) continue;

            foreach ($group->get_group_channels()->get_order() as $channel_id) {
                $channels_order->put($channel_id, $channel_id);
            }
        }

        foreach ($channels_order as $item) {
            $channel = $this->plugin->tv->get_channel($item);
            if (is_null($channel) || $channel->is_disabled()) continue;

            $items[] = Rows_Factory::add_regular_item(
                json_encode(array('group_id' => ALL_CHANNEL_GROUP_ID, 'channel_id' => $channel->get_id())),
                $channel->get_icon_url(),
                $channel->get_title(),
                $this->plugin->get_favorites()->in_order($channel->get_id()) ? $fav_stickers : null
            );
        }

        if (empty($items)) {
            return null;
        }

        return $this->create_rows(
            $items,
            json_encode(array('group_id' => ALL_CHANNEL_GROUP_ID)),
            $all_group->get_title(),
            $all_group->get_title(),
            User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER)
        );
    }

    /**
     * @return array|null
     */
    private function get_regular_rows()
    {
        hd_debug_print(null, true);
        $groups = $this->plugin->tv->get_groups();
        if (is_null($groups))
            return null;

        $rows = array();
        $square_icons =  $this->plugin->get_parameter(PARAM_SQUARE_ICONS);
        $row_item_width = $square_icons ? RowsItemsParams::width_sq : RowsItemsParams::width;

        $fav_stickers[] = Rows_Factory::add_regular_sticker_rect(
            RowsItemsParams::fav_sticker_bg_color,
            Rows_Factory::r(
                $row_item_width - RowsItemsParams::fav_sticker_bg_width - 21,
                0,
                RowsItemsParams::fav_sticker_bg_width,
                RowsItemsParams::fav_sticker_bg_width));

        $fav_stickers[] = Rows_Factory::add_regular_sticker_image(
            get_image_path(RowsItemsParams::fav_sticker_icon_url),
            Rows_Factory::r(
                $row_item_width - RowsItemsParams::fav_sticker_icon_width - 23,
                2,
                RowsItemsParams::fav_sticker_icon_width,
                RowsItemsParams::fav_sticker_icon_height));

        /** @var Group $group */
        /** @var Channel $channel */
        foreach ($this->plugin->tv->get_groups() as $group) {
            if (is_null($group)) continue;

            $items = array();
            foreach ($group->get_group_channels() as $channel) {
                if (is_null($channel) || $channel->is_disabled()) continue;

                $items[] = Rows_Factory::add_regular_item(
                    json_encode(array('group_id' => $group->get_id(), 'channel_id' => $channel->get_id())),
                    $channel->get_icon_url(),
                    $channel->get_title(),
                    $this->plugin->get_favorites()->in_order($channel->get_id()) ? $fav_stickers : null
                );
            }

            if (empty($items)) continue;

            $action_enter = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER);
            $new_rows = $this->create_rows($items,
                json_encode(array('group_id' => $group->get_id())),
                $group->get_title(),
                $group->get_title(),
                $action_enter
            );

            foreach ($new_rows as $row) {
                $rows[] = $row;
            }
        }

        //hd_debug_print("Regular rows: " . count($rows));
        return $rows;
    }

    private function create_rows($items, $row_id, $title, $caption, $action, $color = null)
    {
        $rows = array();
        $rows[] = Rows_Factory::title_row(
            $row_id,
            $caption,
            $row_id,
            TitleRowsParams::width, TitleRowsParams::height,
            is_null($color) ? TitleRowsParams::def_caption_color : $color,
            TitleRowsParams::font_size,
            TitleRowsParams::left_padding,
            0, 0,
            TitleRowsParams::fade_enabled,
            TitleRowsParams::fade_color,
            TitleRowsParams::lite_fade_color);

        for ($i = 0, $iMax = count($items); $i < $iMax; $i += PaneParams::max_items_in_row) {
            $row_items = array_slice($items, $i, PaneParams::max_items_in_row);
            $rows[] = Rows_Factory::regular_row(
                json_encode(array('row_ndx' => (int)($i / PaneParams::max_items_in_row), 'row_id' => $row_id)),
                $row_items,
                'common',
                null,
                $title,
                $row_id,
                RowsParams::width,
                RowsParams::height,
                RowsParams::height - TitleRowsParams::height,
                RowsParams::left_padding,
                RowsParams::inactive_left_padding,
                RowsParams::right_padding,
                RowsParams::hide_captions,
                false,
                RowsParams::fade_enable,
                null,
                $action,
                RowsParams::fade_icon_mix_color,
                RowsParams::fade_icon_mix_alpha,
                RowsParams::lite_fade_icon_mix_alpha,
                RowsParams::fade_caption_color
            );
        }

        return $rows;
    }
}

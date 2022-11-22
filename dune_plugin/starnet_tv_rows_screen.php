<?php
require_once 'lib/tv/ext_epg_program.php';

require_once 'lib/epfs/abstract_rows_screen.php';
require_once 'lib/epfs/rows_factory.php';
require_once 'lib/epfs/gcomps_factory.php';
require_once 'lib/epfs/gcomp_geom.php';
require_once 'lib/epfs/playback_points.php';

class Starnet_Tv_Rows_Screen extends Abstract_Rows_Screen implements User_Input_Handler
{
    const ID = 'rows_epf';

    ///////////////////////////////////////////////////////////////////////////

    private $remove_playback_point;
    private $removed_playback_point;
    private $clear_playback_points = false;

    private $images_path;

    public $need_update_epf_mapping_flag = false;

    ///////////////////////////////////////////////////////////////////////////

    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct(self::ID, $plugin);

        $this->images_path = get_install_path('img');
        Playback_Points::init();
    }

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
        $group_id = isset($media_url->group_id) ? $media_url->group_id : null;
        $channel_id = isset($media_url->channel_id) ? $media_url->channel_id : null;

        if (is_null($channel_id) || empty($group_id))
            return null;

        $channel = $this->plugin->tv->get_channel($channel_id);
        if (is_null($channel)) {
            hd_print("Unknown channel $channel_id");
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

        if (is_null($epg_data = $this->plugin->tv->get_program_info($channel_id, time(), $plugin_cookies))) {

            $channel_desc = $channel->get_desc();
            if (!empty($channel_desc)) {
                $geom = GComp_Geom::place_top_left(PaneParams::info_width, -1, 0, $y);
                $defs[] = GComps_Factory::label($geom,
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
            $program->time = strftime('Длительность: %H:%M', $epg_data[PluginTvEpgProgram::end_tm_sec] - $epg_data[PluginTvEpgProgram::start_tm_sec]);
            //$program->year = preg_match('/\s+\((\d{4,4})\)$/', $epg_data[Ext_Epg_Program::main_category], $matches) ? $matches[1] : '';
            //$program->age = preg_match('/\s+\((\d{1,2}\+)\)$/', $epg_data[Ext_Epg_Program::main_category], $matches) ? $matches[1] : '';

            $title = $epg_data[PluginTvEpgProgram::name];
            $desc = (!empty($epg_data[Ext_Epg_Program::sub_title]) ? $epg_data[Ext_Epg_Program::sub_title] . "\n" : '') . $epg_data[PluginTvEpgProgram::description];
            $fanart_url = '';

            // duration
            $geom = GComp_Geom::place_top_left(PaneParams::info_width, PaneParams::prog_item_height, 0, $y);
            $defs[] = GComps_Factory::label($geom, null, $program->time, 1, PaneParams::prog_item_font_color, PaneParams::prog_item_font_size);
            $y += PaneParams::prog_item_height;

            ///////////// Program title ////////////////

            if (!empty($title)) {
                $lines = array_slice(explode("\n",
                    iconv('Windows-1251', 'UTF-8',
                        wordwrap(iconv('UTF-8', 'Windows-1251',
                            trim(preg_replace('/(!|\?)\.+\s*$/Uu', '$1', $title))),
                            40, "\n", true)
                    )),
                    0, 2);

                $prog_title = implode("\n", $lines);

                if (strlen($prog_title) < strlen($title))
                    $prog_title = $title;

                $lines = min(2, count($lines));
                $geom = GComp_Geom::place_top_left(PaneParams::info_width + 100, PaneParams::prog_item_height, 0, $y + ($lines > 1 ? 20 : 0));
                $defs[] = GComps_Factory::label($geom,
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
                $geom = GComp_Geom::place_top_left(PaneParams::info_width, -1, 0, $y + 5);
                $defs[] = GComps_Factory::label($geom,
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
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(510, 4, 0, 590), null, '#1919BE9F');

        $dy_icon = 530;
        $dy_txt = $dy_icon - 4;
        $dx = 15;
        if ($group_id !== Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID) {

            // blue button image (D)
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon), null, PaneParams::fav_button_blue);

            $dx += 55;
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                1,
                PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );
        } else {

            $fav_channels = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
            $is_first_channel = ($channel_id === reset($fav_channels));
            // green button image (B) 52x50
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
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
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                'Влево',
                1,
                $is_first_channel ? PaneParams::fav_btn_disabled_font_color : PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );

            $is_last_channel = ($channel_id === end($fav_channels));
            $dx += 105;
            // yellow button image (C)
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
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
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                'Вправо',
                1,
                $is_last_channel ? PaneParams::fav_btn_disabled_font_color : PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );

            $dx += 105;
            // blue button image (D)
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon), null, PaneParams::fav_button_blue);

            $dx += 55;
            // blue button text
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt), // label
                null,
                'Удалить',
                1,
                PaneParams::fav_btn_font_color,
                PaneParams::fav_btn_font_size
            );
        }

        ///////////// Enclosing panel ////////////////

        $pane_def = GComps_Factory::get_panel_def('info_pane',
            GComp_Geom::place_top_left(PaneParams::pane_width, PaneParams::pane_height),
            null,
            $defs,
            GCOMP_OPT_PREPAINT
        );
        GComps_Factory::add_extra_var($pane_def, 'info_inf_dimmed', null, array('alpha' => 64));

        return array
        (
            'defs' => array($pane_def),
            'fanart_url' => empty($fanart_url) ? '' : $fanart_url,
        );
    }

    /**
     * @throws Exception
     */
    public function get_folder_view_for_epf(&$plugin_cookies)
    {
        $media_url = MediaURL::decode(self::ID);
        $this->plugin->tv->get_tv_info($media_url, $plugin_cookies);

        return $this->get_folder_view($media_url, $plugin_cookies);
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
     * @throws Exception
     */
    public function get_rows_pane(MediaURL $media_url, $plugin_cookies)
    {
        $rows = $this->get_regular_rows($plugin_cookies);
        if (is_null($rows)) {
            hd_print("no rows");
            return null;
        }

        $favorites_rows = $this->get_favorites_rows($plugin_cookies);
        if (!is_null($favorites_rows))
            $rows = array_merge($favorites_rows, $rows);

        $history_rows = $this->get_history_rows($plugin_cookies);
        if (!is_null($history_rows))
            $rows = array_merge($history_rows, $rows);

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

        $def_params = Rows_Factory::variable_params(
            RowsItemsParams::width,
            RowsItemsParams::height,
            0,
            RowsItemsParams::icon_width,
            RowsItemsParams::icon_height,
            5,
            RowsItemsParams::caption_dy,
            RowsItemsParams::def_caption_color,
            RowsItemsParams::caption_font_size
        );

        $icon_dx = RowsItemsParams::icon_width / RowsItemsParams::icon_height;
        $icon_width = RowsItemsParams::icon_width + 12;
        $sel_params = Rows_Factory::variable_params(
            RowsItemsParams::width,
            RowsItemsParams::height,
            5,
            $icon_width,
            round($icon_width / $icon_dx),
            0,
            RowsItemsParams::caption_dy + 10,
            RowsItemsParams::sel_caption_color,
            RowsItemsParams::caption_font_size
        );

        $width = round((RowsItemsParams::width * 7 - 350) / 7);
        $icon_width = round((RowsItemsParams::icon_width * 7 - 350) / 7) + round((RowsItemsParams::width - RowsItemsParams::icon_width) / $icon_dx);
        $inactive_params = Rows_Factory::variable_params(
            $width, round($width / RowsItemsParams::width * RowsItemsParams::height), 0,
            $icon_width,
            round($icon_width / $icon_dx),
            0,
            RowsItemsParams::caption_dy,
            RowsItemsParams::inactive_caption_color,
            RowsItemsParams::caption_font_size
        );

        $params = Rows_Factory::item_params(
            $def_params,
            $sel_params,
            $inactive_params,
            $this->images_path . RowsItemsParams::icon_loading_url,
            $this->images_path . RowsItemsParams::icon_loading_failed_url,
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
            GUI_EVENT_KEY_ENTER => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER),
            GUI_EVENT_KEY_B_GREEN => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_MOVE_UP),
            GUI_EVENT_KEY_C_YELLOW => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_MOVE_DOWN),
            GUI_EVENT_KEY_D_BLUE => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_ADD),
            GUI_EVENT_KEY_CLEAR => User_Input_Handler_Registry::create_action($this, PLUGIN_FAVORITES_OP_REMOVE),
            GUI_EVENT_KEY_POPUP_MENU => User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_POPUP_MENU),
            GUI_EVENT_PLUGIN_ROWS_INFO_UPDATE => User_Input_Handler_Registry::create_action($this, GUI_EVENT_PLUGIN_ROWS_INFO_UPDATE),
        );
    }

    /**
     * @throws Exception
     */
    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Starnet_Tv_Rows_Screen: handle_user_input');
        //foreach ($user_input as $key => $value) hd_print("  $key => $value");

        if (isset($user_input->item_id)) {
            $media_url_str = $user_input->item_id;
            $media_url = MediaURL::decode($media_url_str);
        } else {
            $media_url = $this->get_parent_media_url($user_input->parent_sel_state);
            $media_url_str = '';
            if (is_null($media_url))
                return null;
        }

        $control_id = $user_input->control_id;

        switch ($control_id) {
            case GUI_EVENT_TIMER:
                // rising after playback end + 100 ms
                Playback_Points::update();
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders();

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
                    empty($info_children['fanart_url']) ? $this->images_path . PaneParams::vod_bg_url : $info_children['fanart_url'],
                    $this->images_path . PaneParams::vod_bg_url,
                    $this->images_path . PaneParams::vod_mask_url,
                    array("plugin_tv://" . get_plugin_name() . "/$user_input->item_id")
                );

            case GUI_EVENT_KEY_POPUP_MENU:
                if ($media_url->group_id === Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID) {
                    if (isset($user_input->selected_item_id)) {
                        $menu_items[] = array(
                            GuiMenuItemDef::caption => '  Удалить',
                            GuiMenuItemDef::icon_url => $this->images_path . '/remove.png',
                            GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'remove_playback_point'),
                        );
                    }

                    $menu_items[] = array(
                        GuiMenuItemDef::caption => '  Очистить',
                        GuiMenuItemDef::icon_url => $this->images_path . '/brush.png',
                        GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'clear_playback_points'),
                    );
                    $menu_items[] = array(GuiMenuItemDef::is_separator => true,);
                }

                $menu_items[] = array(
                    GuiMenuItemDef::caption => '  Обновить',
                    GuiMenuItemDef::icon_url => $this->images_path . '/refresh.png',
                    GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'refresh_screen'),
                );

                return Action_Factory::show_popup_menu($menu_items);

            case PLUGIN_FAVORITES_OP_ADD:
            case PLUGIN_FAVORITES_OP_REMOVE:
                if (!isset($media_url->group_id) || $media_url->group_id === Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID)
                    break;

                $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
                $is_in_favorites = in_array($media_url->channel_id, $fav_channel_ids);

                if ($control_id === PLUGIN_FAVORITES_OP_ADD) {
                    $control_id = $is_in_favorites ? PLUGIN_FAVORITES_OP_REMOVE : PLUGIN_FAVORITES_OP_ADD;
                }

                return $this->plugin->tv->change_tv_favorites($control_id, $media_url->channel_id, $plugin_cookies);

            case PLUGIN_FAVORITES_OP_MOVE_UP:
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                if (!isset($media_url->group_id) || $media_url->group_id !== Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID)
                    break;

                return $this->plugin->tv->change_tv_favorites($control_id, $media_url->channel_id, $plugin_cookies);

            case 'refresh_screen':
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case 'remove_playback_point':
                $this->removed_playback_point = $media_url->get_raw_string();
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case 'clear_playback_points':
                $this->clear_playback_points = true;
                Starnet_Epfs_Handler::update_all_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();
        }

        return null;
    }

    ////////////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return array
     */
    private function get_history_rows($plugin_cookies)
    {
        //hd_print("Starnet_Tv_Rows_Screen::get_history_rows");
        if ($this->clear_playback_points) {
            Playback_Points::clear();
            $this->clear_playback_points = false;
            return null;
        }

        // Fill view history data
        $now = time();
        $rows = array();
        $watched = array();
        foreach (Playback_Points::get_all() as $channel_id => $channel_ts) {
            if (is_null($channel = $this->plugin->tv->get_channel($channel_id))) continue;

            $prog_info = $this->plugin->tv->get_program_info($channel_id, $channel_ts, $plugin_cookies);
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

            //hd_print("Starnet_Tv_Rows_Screen::get_history_rows: channel id: $channel_id (epg: '$title') time mark: $channel_ts progress: " . $progress * 100 . "%");
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
                $id = json_encode(array('group_id' => Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID, 'channel_id' => $item['channel_id'], 'archive_tm' => $item['archive_tm']));
                //hd_print("MediaUrl info for {$item['channel_id']} - $id");
                if (isset($this->removed_playback_point))
                    if ($this->removed_playback_point === $id) {
                        $this->removed_playback_point = null;
                        Playback_Points::clear($item['channel_id']);
                        continue;
                    }

                $stickers = null;

                if ($item['view_progress'] > 0) {
                    // item size 229x142
                    if (!empty($item['program_icon_url'])) {
                        // add small channel logo
                        $rect = Rows_Factory::r(129, 0, 100, 64);
                        Rows_Factory::add_regular_sticker_rect($stickers, "#FFFFFFFF", $rect);
                        Rows_Factory::add_regular_sticker_image($stickers, $channel->get_icon_url(), $rect);
                    }

                    // add progress indicator
                    $progress_y = RowsItemsParams::height - 96;
                    $progress_w = RowsItemsParams::width - 22;
                    Rows_Factory::add_regular_sticker_rect(
                        $stickers,
                        "#6A6A6ACF",
                        Rows_Factory::r(0, $progress_y, $progress_w, 8)); // total

                    Rows_Factory::add_regular_sticker_rect(
                        $stickers,
                        "#EFAA16FF",
                        Rows_Factory::r(0, $progress_y, round($progress_w * $item['view_progress']), 8)); // viewed
                }

                Rows_Factory::add_regular_item(
                    $items,
                    $id,
                    $channel->get_icon_url(),
                    $item['program_title'],
                    $stickers);
            }
        }

        // create view history group
        if (!empty($items)) {
            $new_rows = $this->create_rows($items,
                json_encode(array('group_id' => Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID)),
                'Продолжить',
                'Продолжить просмотр',
                null,
                TitleRowsParams::history_caption_color
            );

            foreach ($new_rows as $row) {
                $rows[] = $row;
            }
        }

        hd_print("History rows: " . count($rows));
        return $rows;
    }

    /**
     * @param $plugin_cookies
     * @return array|null
     */
    private function get_favorites_rows($plugin_cookies)
    {
        //hd_print("Starnet_Tv_Rows_Screen::get_favorites_rows");
        $groups = $this->plugin->tv->get_groups();
        if (is_null($groups))
            return null;

        $rows = array();
        $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
        foreach ($groups as $group) {
            $items = array();

            if (!$group->is_favorite_group()) continue;
            // in favorite group channels not stored! only id's
            $fav_idx = 0;
            $fav_count = count($fav_channel_ids);

            foreach ($fav_channel_ids as $channel_id) {
                $channel = $this->plugin->tv->get_channel($channel_id);
                if (is_null($channel)) continue;

                Rows_Factory::add_regular_item(
                    $items,
                    json_encode(array('group_id' => $group->get_id(), 'channel_id' => $channel->get_id(), 'fav_idx' => "$fav_idx/$fav_count")),
                    $channel->get_icon_url(),
                    $channel->get_title()
                );
                $fav_idx++;
            }

            if (!empty($items)) {
                $action_enter = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER);
                $new_rows = $this->create_rows($items,
                    json_encode(array('group_id' => $group->get_id())),
                    $group->get_title(),
                    $group->get_title(),
                    $action_enter,
                    TitleRowsParams::fav_caption_color
                );

                foreach ($new_rows as $row) {
                    $rows[] = $row;
                }
            }
        }

        hd_print("Favorites rows: " . count($rows));
        return $rows;
    }

    /**
     * @param $plugin_cookies
     * @return array|null
     */
    private function get_regular_rows($plugin_cookies)
    {
        //hd_print("Starnet_Tv_Rows_Screen::get_regular_rows");
        $groups = $this->plugin->tv->get_groups();
        if (is_null($groups))
            return null;

        $rows = array();
        $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
        foreach ($groups as $group) {
            if ($group->is_favorite_group()) continue;

            $items = array();
            $fav_stickers = null;
            Rows_Factory::add_regular_sticker_rect(
                $fav_stickers,
                RowsItemsParams::fav_sticker_bg_color,
                Rows_Factory::r(229 - 40, 0, 40, 40));

            Rows_Factory::add_regular_sticker_image(
                $fav_stickers,
                $this->images_path . RowsItemsParams::fav_sticker_icon_url,
                Rows_Factory::r(229 - 38, 2, 36, 36));

            foreach ($group->get_group_channels() as $channel) {
                $is_in_fav = in_array($channel->get_id(), $fav_channel_ids) ? $fav_stickers : null;
                Rows_Factory::add_regular_item(
                    $items,
                    json_encode(array('group_id' => $group->get_id(), 'channel_id' => $channel->get_id())),
                    $channel->get_icon_url(),
                    $channel->get_title(),
                    $is_in_fav
                );
            }

            if (!empty($items)) {
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
        }

        hd_print("Regular rows: " . count($rows));
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

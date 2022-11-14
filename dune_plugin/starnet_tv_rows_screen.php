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

        ///////////// Channel number & title /////////////////

        $number = $channel->get_number();
        $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(150, 50, 1630 - min(3, strlen($number)) * 20),
            null,
            $number,
            1,
            PaneParams::ch_num_font_color,
            PaneParams::ch_num_font_size,
            'ch_number'
        );

        $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width + 400, PaneParams::prog_item_height),
            null,
            $channel->get_title(),
            1,
            PaneParams::ch_title_font_color,
            PaneParams::ch_title_font_size,
            'ch_title'
        );
        $y = PaneParams::prog_item_height + 20;

        ///////////// start_time, end_time, genre, country, person /////////////////

        $str = '';

        if (!is_null($epg_data = $this->plugin->tv->get_program_info($channel_id, time(), $plugin_cookies))) {
            $program = (object)array();
            $program->time = strftime('%H:%M', $epg_data[PluginTvEpgProgram::end_tm_sec] - $epg_data[PluginTvEpgProgram::start_tm_sec]);
            $program->year = $epg_data[Ext_Epg_Program::year];
            $program->age = preg_match('/\[(.*)\]\s*(.*)$/', $epg_data[Ext_Epg_Program::main_category], $matches) ? $matches[1] : '';
            $program->genre = isset($matches[2]) ? $matches[2] : '';
            $program->country = '';
            $title = preg_replace('/^[ДТХ]\/[фс]\s+/ui', '', $epg_data[PluginTvEpgProgram::name]);
            $desc = (!empty($epg_data[Ext_Epg_Program::sub_title]) ? $epg_data[Ext_Epg_Program::sub_title] . "\n" : '') . $epg_data[PluginTvEpgProgram::description];
            $fanart_url = preg_replace('/\/c400x300\//', '/c400x248/', $epg_data[PluginTvEpgProgram::icon_url]);

            foreach (array('time', 'genre', 'country', 'year', 'age') as $key) {
                $val = $program->{$key};

                if (!empty($val)) {
                    if ($str)
                        $str .= ' | ';

                    $str .= $val;
                }
            }

            if ($str) {
                $geom = GComp_Geom::place_top_left(PaneParams::info_width, PaneParams::prog_item_height, 0, $y);
                $defs[] = GComps_Factory::label($geom, null, $str, 1, PaneParams::prog_item_font_color, PaneParams::prog_item_font_size);
                $y += PaneParams::prog_item_height + 12;
            }

            ///////////// Program title ////////////////

            if (!empty($title)) {
                $lines = array_slice(explode("\n",
                    iconv('Windows-1251', 'UTF-8',
                        wordwrap(iconv('UTF-8', 'Windows-1251',
                            trim(preg_replace('/(!|\?)\.+\s*$/Uu', '$1', $title))),
                            40, "\n", true)
                    )), 0, 2);

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
                $y += PaneParams::prog_item_height * $lines + ($lines > 1 ? 10 : 0);
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
        } else {
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
        }

        $dx = 15;
        $dy_icon = 530;
        $dy_txt = $dy_icon - 4;
        $fav_channels = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(710, 54, 0, $dy_icon - 4), null, '#9F5843A6');
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(984, 6, 710, $dy_icon - 4 + 48), null, '#9F5843A6');
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(6, 6, 1700, $dy_icon - 4 + 48), null, '#9F5843A6');
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(6, 6, 1712, $dy_icon - 4 + 48), null, '#9F5843A6');
        $defs[] = GComps_Factory::get_rect_def(GComp_Geom::place_top_left(6, 6, 1724, $dy_icon - 4 + 48), null, '#9F5843A6');

        if ($group_id === Default_Dune_Plugin::FAV_CHANNEL_GROUP_ID) {
            $first = ($channel_id === reset($fav_channels));
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
                null,
                'gui_skin://special_icons/controls_button_green.aai',
                false,
                true,
                null,
                null,
                null,
                $first ? 99 : 255);

            $dx += 52;
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt),
                null,
                'Вперед',
                1,
                $first ? '#FF808080' : '#FFE0E0E0',
                PaneParams::prog_item_font_size - 4
            );

            $last = ($channel_id === end($fav_channels));
            $dx += 104;
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
                null,
                'gui_skin://special_icons/controls_button_yellow.aai',
                1,
                0,
                null,
                null,
                null,
                $last ? 99 : 255
            );

            $dx += 52;
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt),
                null,
                'Назад',
                1,
                $last ? '#FF707070' : '#FFE0E0E0',
                PaneParams::prog_item_font_size - 4
            );
            $dx += 90;
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
                null, 'gui_skin://special_icons/controls_button_blue.aai');
            $dx += 52;
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt),
                null,
                'Удалить',
                1,
                '#FFE0E0E0',
                PaneParams::prog_item_font_size - 4
            );
        } else {
            $defs[] = GComps_Factory::get_image_def(GComp_Geom::place_top_left(52, 50, $dx, $dy_icon),
                null, 'gui_skin://special_icons/controls_button_blue.aai');
            $dx += 52;
            $defs[] = GComps_Factory::label(GComp_Geom::place_top_left(PaneParams::info_width, -1, $dx, $dy_txt),
                null,
                Default_Dune_Plugin::FAV_CHANNEL_GROUP_CAPTION,
                1,
                '#FFE0E0E0',
                PaneParams::prog_item_font_size - 4
            );
        }

        ///////////// Enclosing panel ////////////////

        $pane_def = GComps_Factory::get_panel_def('info_pane',
            GComp_Geom::place_top_left(1820, PaneParams::info_height - PaneParams::dx),
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
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_rows_pane(MediaURL $media_url, &$plugin_cookies)
    {
        $pane = $this->create_rows_pane($plugin_cookies);
        if (is_null($pane)) {
            hd_print("no panes");
            return null;
        }

        $rows = array();
        $min_row_index_for_y2 = 1;

        if ($this->clear_playback_points) {
            Playback_Points::clear();
            $this->clear_playback_points = false;
        } else {
            $watched = array();

            foreach (Playback_Points::get_all() as $point) {
                $channel_id = $point->channel_id;

                if (is_null($channel = $this->plugin->tv->get_channel($channel_id)) || $channel->is_protected())
                    continue;

                $channel_ts = ($point->archive_tm > 0) ?
                    $point->archive_tm + $point->position : // archive
                    ($channel->has_archive() ? $point->time : 0); // if can be archived current position

                if (isset($watched[$channel_id]))
                    continue;

                if ($channel_ts < 0) {
                    // only live stream
                    $watched[(string)$channel_id] = array
                    (
                        'channel_id' => $channel_id,
                        'archive_tm' => 0,
                        'view_progress' => 0,
                        'program_title' => $channel->get_title(),
                        'program_icon_url' => '',
                    );
                } else if (!is_null($prog_info = $this->plugin->tv->get_program_info($channel_id, $channel_ts, $plugin_cookies))) {
                    // program epg available
                    $start_tm = $prog_info[PluginTvEpgProgram::start_tm_sec];
                    $end_tm = $prog_info[PluginTvEpgProgram::end_tm_sec];
                    $subtitle = isset($prog_info[Ext_Epg_Program::sub_title]) ? $prog_info[Ext_Epg_Program::sub_title] : '';

                    $watched[(string)$channel_id] = array
                    (
                        'channel_id' => $channel_id,
                        'archive_tm' => $channel_ts,
                        'view_progress' => max(0.01, min(1, 1 - round(($end_tm - $channel_ts) / ($end_tm - $start_tm), 2))),
                        'program_title' => $prog_info[PluginTvEpgProgram::name] . (empty($subtitle) ? '' : '. ' . $subtitle),
                        'program_icon_url' => isset($prog_info[Ext_Epg_Program::main_icon]) ? $prog_info[Ext_Epg_Program::main_icon] : '',
                    );
                } else {
                    $watched[(string)$channel_id] = array
                    (
                        'channel_id' => $channel_id,
                        'archive_tm' => $channel_ts,
                        'view_progress' => 0,
                        'program_title' => '',
                        'program_icon_url' => '',
                    );
                }

                if (count($watched) >= 7)
                    break;
            }

            $items = array();

            foreach ($watched as $item) {
                if (!is_null($channel = $this->plugin->tv->get_channel($item['channel_id']))) {
                    $id = json_encode(array('group_id' => Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID, 'channel_id' => $item['channel_id'], 'archive_tm' => $item['archive_tm']));

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
                        Rows_Factory::add_regular_sticker_rect($stickers, "#CF6A6A6A", Rows_Factory::r(0, 134, 229, 8));
                        Rows_Factory::add_regular_sticker_rect($stickers, "#FFEFAA16", Rows_Factory::r(0, 134, round(229 * $item['view_progress']), 8));
                    }

                    Rows_Factory::add_regular_item(
                        $items,
                        $id,
                        empty($item['program_icon_url']) ? $channel->get_icon_url() : $item['program_icon_url'],
                        empty($item['program_title']) ? $channel->get_title() : $item['program_title'],
                        $stickers);
                }
            }

            if (count($items) < 7) {
                $t = time();
                $epf_data = Starnet_Epfs_Handler::get_epf();

                if (isset($epf_data, $epf_data->data->pane->rows)) {
                    foreach ($epf_data->data->pane->rows as $row) {
                        if ($row->type !== PLUGIN_ROW_TYPE_REGULAR) continue;

                        $id = json_decode($row->id);
                        $row_id = json_decode($id->row_id);

                        if (!isset($row_id->group_id) || ($row_id->group_id !== Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID))
                            continue;

                        foreach ($row->data->items as $item) {
                            $item_id = json_decode($item->id);
                            if (is_null($item_id)) break;

                            if (isset($this->removed_playback_point) && $this->removed_playback_point === $item->id) {
                                $this->removed_playback_point = null;
                                Playback_Points::clear($item->id);
                                continue;
                            }

                            $channel_id = $item_id->channel_id;

                            if (!isset($watched[$channel_id])
                                && !is_null($channel = $this->plugin->tv->get_channel($channel_id))
                                && ($item_id->archive_tm + $channel->get_archive_past_sec() - 60 > $t)) {

                                $items[] = $item;
                                $watched[$channel_id] = '';

                                if (count($items) >= 7)
                                    break 2;
                            }
                            break;
                        }
                    }
                }
            }

            if (!empty($items)) {
                $min_row_index_for_y2 = 2;
                $row_gid = json_encode(array('group_id' => Default_Dune_Plugin::PLAYBACK_HISTORY_GROUP_ID));
                $rows[] = Rows_Factory::title_row($row_gid,
                    'Продолжить просмотр',
                    $row_gid,
                    TitleRowsParams::width,
                    TitleRowsParams::height,
                    '#FFEFAA16',
                    TitleRowsParams::font_size,
                    TitleRowsParams::left_padding,
                    0, 0,
                    TitleRowsParams::fade_enabled,
                    TitleRowsParams::fade_color,
                    TitleRowsParams::lite_fade_color
                );

                $rows[] = Rows_Factory::regular_row(json_encode(array('row_ndx' => 0, 'row_id' => $row_gid)),
                    $items,
                    'common',
                    null,
                    'Продолжить',
                    $row_gid,
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
                    null,
                    RowsParams::fade_icon_mix_color,
                    RowsParams::fade_icon_mix_alpha,
                    RowsParams::lite_fade_icon_mix_alpha,
                    RowsParams::fade_caption_color
                );
            }
        }

        $this->add_rows_to_pane($pane, $rows, null, $min_row_index_for_y2);

        return $pane;
    }

    /**
     * @throws Exception
     */
    private function create_rows_pane(&$plugin_cookies)
    {
        $groups = $this->plugin->tv->get_groups();
        if (is_null($groups))
            return null;

        $fav_channel_ids = $this->plugin->tv->get_fav_channel_ids($plugin_cookies);
        $rows = array();
        foreach ($groups as $group) {
            $items = array();

            if ($group->is_favorite_group()) {
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
            } else {
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
            }

            if (empty($items)) continue;

            $row_gid = json_encode(array('group_id' => $group->get_id()));
            $rows[] = Rows_Factory::title_row(
                $row_gid,
                $group->get_title(),
                $row_gid,
                TitleRowsParams::width, TitleRowsParams::height,
                ($group->is_favorite_group()) ? TitleRowsParams::fav_caption_color : TitleRowsParams::def_caption_color,
                TitleRowsParams::font_size,
                TitleRowsParams::left_padding,
                0, 0,
                TitleRowsParams::fade_enabled,
                TitleRowsParams::fade_color,
                TitleRowsParams::lite_fade_color);

            $action_enter = User_Input_Handler_Registry::create_action($this, GUI_EVENT_KEY_ENTER);

            for ($i = 0, $iMax = count($items); $i < $iMax; $i += PaneParams::max_items_in_row) {
                $rows[] = Rows_Factory::regular_row(
                    json_encode(array('row_ndx' => (int)($i / PaneParams::max_items_in_row), 'row_id' => $row_gid)),
                    array_slice($items, $i, PaneParams::max_items_in_row),
                    'common',
                    null,
                    $group->get_title(),
                    $row_gid,
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
                    $action_enter,
                    RowsParams::fade_icon_mix_color,
                    RowsParams::fade_icon_mix_alpha,
                    RowsParams::lite_fade_icon_mix_alpha,
                    RowsParams::fade_caption_color
                );
            }
        }

        hd_print("total rows: " . count($rows));
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
            1,
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
            $width, round($width / (RowsItemsParams::width / RowsItemsParams::height)), 0,
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
                Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);
                return Starnet_Epfs_Handler::invalidate_folders();

            case GUI_EVENT_KEY_ENTER:
                $tv_play_action = Action_Factory::tv_play($media_url);

                if (isset($user_input->action_origin)) {
                    Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);
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
                Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case 'remove_playback_point':
                $this->removed_playback_point = $media_url->get_raw_string();
                Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case 'clear_playback_points':
                $this->clear_playback_points = true;
                Starnet_Epfs_Handler::update_tv_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();
        }

        return null;
    }
}

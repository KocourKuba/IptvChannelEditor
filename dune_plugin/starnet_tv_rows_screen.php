<?php
require_once 'lib/epfs/playback_points.php';
require_once 'lib/epfs/tv_rows_screen.php';

class Starnet_Tv_Rows_Screen extends Tv_Rows_Screen
{
    ///////////////////////////////////////////////////////////////////////////

    private $remove_playback_point;
    private $removed_playback_point;
    private $clear_playback_points = false;
    private $playback_history_enable;

    ///////////////////////////////////////////////////////////////////////////

    public function __construct(Default_Dune_Plugin $plugin)
    {
        parent::__construct($plugin);

        $this->playback_history_enable = class_exists('Playback_Points');
        if ($this->playback_history_enable) {
            Playback_Points::init();
        }
    }

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     * @throws Exception
     */
    public function get_rows_pane(MediaURL $media_url, &$plugin_cookies)
    {
        hd_print("Starnet_Tv_Rows_Screen::get_rows_pane");
        $pane = parent::get_rows_pane($media_url, $plugin_cookies);
        if (is_null($pane)) {
            hd_print("no panes");
            return null;
        }

        if (!$this->playback_history_enable) {
            hd_print("no playback history enabled");
            return $pane;
        }

        $rows = array();
        $min_row_index_for_y2 = 1;

        if (!$this->clear_playback_points) {
            $watched = array();

            foreach (Playback_Points::get_all() as $point) {
                $channel_id = $point->channel_id;

                hd_print("Playback_Point: for channel $channel_id");
                if (is_null($channel = $this->plugin->tv->get_channel($channel_id)) || $channel->is_protected())
                    continue;

                $channel_ts = ($point->archive_tm > 0) ? $point->archive_tm + $point->position : ($channel->has_archive() ? $point->time : 0);

                if (isset($watched[(string)$channel_id]))
                    continue;

                if (empty($channel_ts)) {
                    $watched[(string)$channel_id] = array
                    (
                        'channel_id' => $channel_id,
                        'archive_tm' => 0,
                        'view_progress' => 0,
                        'program_title' => $channel->get_title(),
                        'program_icon_url' => '',
                    );
                } else if (!is_null($prog_info = $this->plugin->tv->get_program_info($channel_id, $channel_ts, false))) {
                    $start_tm = $prog_info[PluginTvEpgProgram::start_tm_sec];
                    $end_tm = $prog_info[PluginTvEpgProgram::end_tm_sec];
                    $subtitle = $prog_info[Ext_Epg_Program::sub_title];

                    $watched[(string)$channel_id] = array
                    (
                        'channel_id' => $channel_id,
                        'archive_tm' => $channel_ts,
                        'view_progress' => max(0.01, min(1, 1 - round(($end_tm - $channel_ts) / ($end_tm - $start_tm), 2))),
                        'program_title' => $prog_info[PluginTvEpgProgram::name] . (empty($subtitle) ? '' : '. ' . $subtitle),
                        'program_icon_url' => $prog_info[PluginTvEpgProgram::icon_url],
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
                    $id = json_encode(array('group_id' => self::PLAYBACK_HISTORY_GROUP_ID, 'channel_id' => $item['channel_id'], 'archive_tm' => $item['archive_tm']));

                    if (isset($this->removed_playback_point))
                        if ($this->removed_playback_point === $id) {
                            $this->removed_playback_point = null;
                            Playback_Points::clear();
                            continue;
                        }

                    $stickers = null;

                    if (!empty($item['view_progress']) && !empty($item['program_icon_url'])) {
                        $rect = Rows_Factory::r(229 - 100, 0, 100, 64);
                        Rows_Factory::add_regular_sticker_rect($stickers, "#FFFFFFFF", $rect);
                        Rows_Factory::add_regular_sticker_image($stickers, $channel->get_icon_url(), $rect);
                    }

                    if ($item['view_progress'] > 0) {
                        Rows_Factory::add_regular_sticker_rect($stickers, "#CF6A6A6A", Rows_Factory::r(0, 142 - 8, 229, 8));
                        Rows_Factory::add_regular_sticker_rect($stickers, "#FFEFAA16", Rows_Factory::r(0, 142 - 8, round(229 * $item['view_progress']), 8));
                    }

                    Rows_Factory::add_regular_item(
                        $items,
                        $id,
                        empty($stickers) ? $channel->get_icon_url() : $item['program_icon_url'],
                        empty($stickers) ? $channel->get_title() : $item['program_title'],
                        $stickers);
                }
            }

            if (count($items) < 7) {
                $t = time();
                $epf_data = Starnet_Epfs_Handler::get_epf();

                if (isset($epf_data->data->pane->rows)) {
                    foreach ($epf_data->data->pane->rows as $row) {
                        if ($row->type !== PLUGIN_ROW_TYPE_REGULAR) continue;

                        $id = json_decode($row->id);
                        $row_id = json_decode($id->row_id);

                        if (!isset($row_id->group_id) || ($row_id->group_id !== self::PLAYBACK_HISTORY_GROUP_ID))
                            continue;

                        foreach ($row->data->items as $item) {
                            $item_id = json_decode($item->id);
                            if (is_null($item_id)) break;

                            if (isset($this->removed_playback_point) && $this->removed_playback_point === $item->id) {
                                $this->removed_playback_point = null;
                                Playback_Points::clear();
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
                $row_gid = json_encode(array('group_id' => self::PLAYBACK_HISTORY_GROUP_ID));
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
        } else {
            Playback_Points::clear();
            $this->clear_playback_points = false;
        }

        $this->add_rows_to_pane($pane, $rows, null, $min_row_index_for_y2);

        return $pane;
    }

    public function handle_user_input(&$user_input, &$plugin_cookies)
    {
        //hd_print('Starnet_Tv_Rows_Screen: handle_user_input:');
        //foreach ($user_input as $key => $value) hd_print("  $key => $value");

        if (isset($user_input->item_id))
            $media_url = MediaURL::decode($user_input->item_id);
        else if (is_null($media_url = $this->get_parent_media_url($user_input->parent_sel_state)))
            return null;

        switch ($user_input->control_id) {
            case GUI_EVENT_KEY_POPUP_MENU:
                if ($media_url->group_id === self::PLAYBACK_HISTORY_GROUP_ID) {
                    $menu_items[] = array
                        (
                            GuiMenuItemDef::caption => '  Удалить',
                            GuiMenuItemDef::icon_url => $this->images_path . '/remove.png',
                            GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'remove_playback_point'),
                        );
                    $menu_items[] = array
                        (
                            GuiMenuItemDef::caption => '  Очистить',
                            GuiMenuItemDef::icon_url => $this->images_path . '/brush.png',
                            GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'clear_playback_points'),
                        );
                    $menu_items[] = array
                        (
                            GuiMenuItemDef::is_separator => true,
                        );
                }

                $menu_items[] = array
                    (
                        GuiMenuItemDef::caption => '  Обновить',
                        GuiMenuItemDef::icon_url => $this->images_path . '/refresh.png',
                        GuiMenuItemDef::action => User_Input_Handler_Registry::create_action($this, 'refresh_screen'),
                    );

                return Action_Factory::show_popup_menu($menu_items);

            case 'remove_playback_point':
                $this->removed_playback_point = $media_url->get_raw_string();
                $this->clear_playback_points = empty($this->removed_playback_point);
                Starnet_Epfs_Handler::refresh_tv_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case 'clear_playback_points':
                $this->clear_playback_points = empty($this->removed_playback_point);
                Starnet_Epfs_Handler::refresh_tv_epfs($plugin_cookies);

                return Starnet_Epfs_Handler::invalidate_folders();

            case PLUGIN_FAVORITES_OP_ADD:
            case PLUGIN_FAVORITES_OP_REMOVE:
            case PLUGIN_FAVORITES_OP_MOVE_UP:
            case PLUGIN_FAVORITES_OP_MOVE_DOWN:
                if ($media_url->group_id === self::PLAYBACK_HISTORY_GROUP_ID)
                    return null;
        }

        return parent::handle_user_input($user_input, $plugin_cookies);
    }
}

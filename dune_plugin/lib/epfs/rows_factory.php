<?php

class Rows_Factory
{
    public static function pane($rows, $focus = null, $bg = null,
                                $header_enabled = false, $single_list_navigation = false,
                                $initial_focus_header = -1,
                                $initial_focus_item_id = null, $initial_focus_row_id = null,
                                $hfactor = 1.0, $vfactor = 1.0, $vgravity = 0.0, $vend_min_offset = 0)
    {
        if (!$focus)
            $focus = self::focus();
        $arr = array(
            PluginRowsPane::rows => $rows,
            PluginRowsPane::focus => $focus,
            PluginRowsPane::bg => $bg,
            PluginRowsPane::header_enabled => $header_enabled,
            PluginRowsPane::initial_focus_header => $initial_focus_header,
            PluginRowsPane::initial_focus_item_id => $initial_focus_item_id,
            PluginRowsPane::initial_focus_row_id => $initial_focus_row_id,
            PluginRowsPane::horizontal_focus_freedom_factor => $hfactor,
            PluginRowsPane::vertical_focus_freedom_factor => $vfactor,
            PluginRowsPane::vertical_focus_gravity => $vgravity,
            PluginRowsPane::vertical_focus_end_min_offset => $vend_min_offset,
        );
        if (defined('PluginRowsPane::single_list_navigation'))
            $arr[PluginRowsPane::single_list_navigation] = $single_list_navigation;

        return $arr;
    }

    public static function pane_set_geometry(&$pane,
                                             $w, $h, $x, $y, $y2 = 0, $min_row_index_for_y2 = 0,
                                             $info_w = 0, $info_h = 0, $info_x = 0, $info_y = 0,
                                             $vod_w = 0, $vod_h = 0)
    {
        $pane[PluginRowsPane::screen_r] = array('w' => $w, 'h' => $h, 'x' => $x, 'y' => $y);
        $pane[PluginRowsPane::screen_y2] = $y2;
        $pane[PluginRowsPane::min_row_index_for_y2] = $min_row_index_for_y2;
        $pane[PluginRowsPane::info_r] = array('w' => $info_w, 'h' => $info_h, 'x' => $info_x, 'y' => $info_y);
        $pane[PluginRowsPane::vod_r] = array('w' => $vod_w, 'h' => $vod_h, 'x' => $w - $vod_w, 'y' => 0);
    }

    public static function focus(
        $focus_type = GCOMP_FOCUS_SYSTEM, $focus2_type = GCOMP_FOCUS_NONE)
    {
        return array(
            GCompFocusDef::type => $focus_type,
            GCompFocusDef::type2 => $focus2_type);
    }

    public static function vgap_row($height, $inactive_height = -1)
    {
        return array(
            PluginRow::type => PLUGIN_ROW_TYPE_VGAP,
            PluginRow::height => $height,
            PluginRow::inactive_height => $inactive_height,
        );
    }

    public static function gcomps_row($id, $gcomp_defs, $title = null,
                                      $width = -1, $height = -1, $inactive_height = -1,
                                      $ui_state = null)
    {
        return array(
            PluginRow::type => PLUGIN_ROW_TYPE_GCOMPS,
            PluginRow::id => $id,
            PluginRow::title => $title,
            PluginRow::height => $height,
            PluginRow::inactive_height => $inactive_height,
            PluginRow::data => array(
                PluginGCompsRow::defs => $gcomp_defs,
                PluginGCompsRow::ui_state => $ui_state,
                PluginGCompsRow::width => $width,
            ));
    }

    public static function title_row($id, $caption,
                                     $group_id = null, $width = null, $height = null,
                                     $color = null, $font_size = null,
                                     $left = null, $dy = null, $active_dy = null,
                                     $fade_enabled = false, $fade_color = null, $lite_fade_color = null)
    {
        $arr = array(
            PluginRow::type => PLUGIN_ROW_TYPE_TITLE,
            PluginRow::id => $id,
            PluginRow::group_id => $group_id,
            PluginRow::height => $height,
            PluginRow::inactive_height => 0,
            PluginRow::data => array(
                PluginTitleRow::caption => $caption,
                PluginTitleRow::color => $color,
                PluginTitleRow::font_size => $font_size,
                PluginTitleRow::left => $left,
                PluginTitleRow::dy => $dy,
                PluginTitleRow::active_dy => $active_dy,
                PluginTitleRow::width => $width,
                PluginTitleRow::fade_enabled => $fade_enabled,
                PluginTitleRow::fade_color => $fade_color,
                PluginTitleRow::lite_fade_color => $lite_fade_color,
            ));

        if (defined('PluginTitleRow::fade_enabled'))
            $arr[PluginTitleRow::fade_enabled] = $fade_enabled;
        if (defined('PluginTitleRow::fade_color'))
            $arr[PluginTitleRow::fade_color] = $fade_color;
        if (defined('PluginTitleRow::lite_fade_color'))
            $arr[PluginTitleRow::lite_fade_color] = $lite_fade_color;

        return $arr;
    }

    public static function set_item_params_template(&$pane, $id, $params)
    {
        $pane[PluginRowsPane::regular_item_params_templates][$id] = $params;
    }

    public static function regular_row($id, $items,
                                       $params_template_id = null, $params = null, $title = null,
                                       $group_id = null, $width = null, $height = null, $inactive_height = null,
                                       $left_padding = null, $inactive_left_padding = null, $right_padding = null,
                                       $hide_captions = null, $hide_icons = null,
                                       $fade_enabled = null, $focusable = null,
                                       $show_all_action = null,
                                       $fade_icon_mix_color = null,
                                       $fade_icon_mix_alpha = null,
                                       $lite_fade_icon_mix_alpha = null,
                                       $fade_caption_color = null)
    {
        $data = array(PluginRegularRow::items => $items);

        $data[PluginRegularRow::item_params_template_id] = isset($params_template_id) ? $params_template_id : null;
        $data[PluginRegularRow::item_params] = isset($params) ? $params : null;
        $data[PluginRegularRow::width] = isset($width) ? $width : null;
        $data[PluginRegularRow::left_padding] = isset($left_padding) ? $left_padding : null;
        $data[PluginRegularRow::inactive_left_padding] = isset($inactive_left_padding) ? $inactive_left_padding : null;
        $data[PluginRegularRow::right_padding] = isset($right_padding) ? $right_padding : null;
        $data[PluginRegularRow::hide_captions] = isset($hide_captions) ? $hide_captions : null;
        $data[PluginRegularRow::hide_icons] = isset($hide_icons) ? $hide_icons : null;
        $data[PluginRegularRow::fade_enabled] = isset($fade_enabled) ? $fade_enabled : null;
        $data[PluginRegularRow::fade_icon_mix_color] = isset($fade_icon_mix_color) ? $fade_icon_mix_color : null;
        $data[PluginRegularRow::fade_icon_mix_alpha] = isset($fade_icon_mix_alpha) ? $fade_icon_mix_alpha : null;
        $data[PluginRegularRow::lite_fade_icon_mix_alpha] = isset($lite_fade_icon_mix_alpha) ? $lite_fade_icon_mix_alpha : null;
        $data[PluginRegularRow::fade_caption_color] = isset($fade_caption_color) ? $fade_caption_color : null;

        $arr = array(
            PluginRow::type => PLUGIN_ROW_TYPE_REGULAR,
            PluginRow::id => $id,
            PluginRow::data => $data
        );

        $arr[PluginRow::title] = isset($title) ? $title : null;
        $arr[PluginRow::group_id] = isset($group_id) ? $group_id : null;
        $arr[PluginRow::height] = isset($height) ? $height : null;
        $arr[PluginRow::inactive_height] = isset($inactive_height) ? $inactive_height : null;
        $arr[PluginRow::focusable] = isset($focusable) ? $focusable : null;
        $arr[PluginRow::show_all_action] = $show_all_action;

        return $arr;
    }

    public static function add_regular_item(&$items, $id, $icon_url, $caption = null, $stickers = null)
    {
        $arr = array(
            PluginRegularItem::id => $id,
            PluginRegularItem::icon_url => $icon_url
        );

        if (isset($caption))
            $arr[PluginRegularItem::caption] = $caption;
        if (isset($stickers))
            $arr[PluginRegularItem::stickers] = $stickers;

        $items[] = $arr;
    }

    public static function variable_params($width, $height, $dx = null,
                                           $icon_width = null, $icon_height = null, $icon_dy = null,
                                           $caption_dy = null, $caption_color = null, $caption_font_size = null,
                                           $sticker_width = null, $sticker_height = null)
    {
        $arr = array(
            PluginRegularItemVariableParams::width => $width,
            PluginRegularItemVariableParams::height => $height);
        if (isset($dx))
            $arr[PluginRegularItemVariableParams::dx] = $dx;
        if (isset($icon_width))
            $arr[PluginRegularItemVariableParams::icon_width] = $icon_width;
        if (isset($icon_height))
            $arr[PluginRegularItemVariableParams::icon_height] = $icon_height;
        if (isset($icon_dy))
            $arr[PluginRegularItemVariableParams::icon_dy] = $icon_dy;
        if (isset($caption_dy))
            $arr[PluginRegularItemVariableParams::caption_dy] = $caption_dy;
        if (isset($caption_color))
            $arr[PluginRegularItemVariableParams::caption_color] = $caption_color;
        if (isset($caption_font_size))
            $arr[PluginRegularItemVariableParams::caption_font_size] = $caption_font_size;
        if (isset($sticker_width))
            $arr[PluginRegularItemVariableParams::sticker_width] = $sticker_width;
        if (isset($sticker_height))
            $arr[PluginRegularItemVariableParams::sticker_height] = $sticker_height;
        return $arr;
    }

    public static function margins($l, $t, $r, $b)
    {
        return array(
            PluginMargins::left => $l,
            PluginMargins::top => $t,
            PluginMargins::right => $r,
            PluginMargins::bottom => $b);
    }

    public static function item_params($def, $sel = null, $inactive = null,
                                       $loading_url = null, $load_failed_url = null,
                                       $caption_max_num_lines = null, $caption_line_spacing = null,
                                       $sel_margins = null)
    {
        $arr = array(
            PluginRegularItemParams::def => $def);
        if (isset($sel))
            $arr[PluginRegularItemParams::sel] = $sel;
        if (isset($inactive))
            $arr[PluginRegularItemParams::inactive] = $inactive;
        if (isset($loading_url))
            $arr[PluginRegularItemParams::loading_url] = $loading_url;
        if (isset($load_failed_url))
            $arr[PluginRegularItemParams::load_failed_url] = $load_failed_url;
        if (isset($caption_max_num_lines))
            $arr[PluginRegularItemParams::caption_max_num_lines] = $caption_max_num_lines;
        if (isset($caption_line_spacing))
            $arr[PluginRegularItemParams::caption_line_spacing] = $caption_line_spacing;
        if (isset($sel_margins))
            $arr[PluginRegularItemParams::sel_margins] = $sel_margins;
        return $arr;
    }

    public static function r($x, $y, $w, $h)
    {
        return array('w' => $w, 'h' => $h, 'x' => $x, 'y' => $y);
    }

    public static function add_regular_sticker_image(&$stickers, $icon_url, $r)
    {
        $stickers[] = array(
            PluginRegularSticker::r => $r,
            PluginRegularSticker::icon_url => $icon_url);
    }

    public static function add_regular_sticker_text(&$stickers, $text, $r)
    {
        $stickers[] = array(
            PluginRegularSticker::r => $r,
            PluginRegularSticker::text => $text);
    }

    public static function add_regular_sticker_rect(&$stickers, $color, $r)
    {
        $stickers[] = array(
            PluginRegularSticker::r => $r,
            PluginRegularSticker::color => $color);
    }
}

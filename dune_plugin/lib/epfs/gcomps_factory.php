<?php

class GComps_Factory
{
    public static function get_label_def($geom, $margins,
        $text, $max_num_lines=1, $color=15, $font_size=FONT_SIZE_NORMAL, $id=null)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_LABEL,
            GComponentDef::specific_def =>
                array(
                    GCompLabelDef::text => $text,
                    GCompLabelDef::max_num_lines => $max_num_lines,
                    GCompLabelDef::text_color => $color,
                    GCompLabelDef::font_size => $font_size,
                ),
        );
    }

    /**
     * @param $attrs
     * @param $text
     * @param $color
     * @param $font_size
     * @param $max_num_lines
     * @return array
     */
    protected static function getSpecificDefs($attrs, $text, $color, $font_size, $max_num_lines)
    {
        $line_spacing = isset($attrs['line_spacing']) ? $attrs['line_spacing'] : null;
        $base_font_size = isset($attrs['base_font_size']) ? $attrs['base_font_size'] : null;
        $halign = isset($attrs['halign']) ? $attrs['halign'] : null;

        $arr2 = array(
            GCompTtfLabelDef::text => $text,
            GCompTtfLabelDef::text_color => $color,
            GCompTtfLabelDef::font_size => $font_size,
        );
        if (isset($max_num_lines) && $max_num_lines !== 1)
            $arr2[GCompTtfLabelDef::max_num_lines] = $max_num_lines;
        if (isset($line_spacing))
            $arr2[GCompTtfLabelDef::line_spacing] = $line_spacing;
        if ($base_font_size)
            $arr2[GCompTtfLabelDef::base_font_size] = $base_font_size;
        if ($halign)
            $arr2[GCompTtfLabelDef::halign] = $halign;
        return $arr2;
    }

    public static function label($geom, $margins,
        $text, $max_num_lines=1, $color="#FFFFFFE0",
        $font_size=36, $id=null, $attrs=null)
    {
        $arr2 = self::getSpecificDefs($attrs, $text, $color, $font_size, $max_num_lines);
        $arr = array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::kind => GCOMPONENT_TTF_LABEL,
            GComponentDef::specific_def => $arr2,
        );
        if ($margins)
            $arr[GComponentDef::margins_def] = $margins;
        return $arr;
    }

    public static function label_v2($geom, $margins,
        $text, $max_num_lines=1, $color="#FFFFFFE0",
        $font_size=36, $id=null, $attrs=null)
    {
        $arr2 = self::getSpecificDefs($attrs, $text, $color, $font_size, $max_num_lines);

        $arr = array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::options => GCOMP_OPT_TTF_LAYOUT_FIX,
            GComponentDef::kind => GCOMPONENT_TTF_LABEL,
            GComponentDef::specific_def => $arr2,
        );
        if ($margins)
            $arr[GComponentDef::margins_def] = $margins;
        return $arr;
    }

    public static function get_rect_def($geom, $margins, $color)
    {
        return array(
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_RECT,
            GComponentDef::specific_def =>
                array(
                    GCompRectDef::color => $color,
                ),
        );
    }

    public static function get_image_def($geom, $margins, $url,
        $keep_aspect_ratio=false, $upscale_enabled=true,
        $not_loaded_url=null, $load_failed_url=null, $low_quality_url=null,
        $alpha=null, $mix_alpha=null, $mix_color=null)
    {
        return array(
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_IMAGE,
            GComponentDef::specific_def =>
                array(
                    GCompImageDef::url => $url,
                    GCompImageDef::keep_aspect_ratio => $keep_aspect_ratio,
                    GCompImageDef::upscale_enabled => $upscale_enabled,
                    GCompImageDef::not_loaded_url => $not_loaded_url,
                    GCompImageDef::load_failed_url => $load_failed_url,
                    GCompImageDef::low_quality_url => $low_quality_url,
                    GCompImageDef::alpha => $alpha,
                    GCompImageDef::mix_alpha => $mix_alpha,
                    GCompImageDef::mix_color => $mix_color,
                ),
        );
    }

    public static function get_cut_image_def($id,
        $geom, $margins, $url)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_CUT_IMAGE,
            GComponentDef::specific_def =>
                array(
                    GCompImageDef::url => $url,
                ),
        );
    }

    public static function get_button_def($id,
        $geom, $margins, $caption, $selected=false, $caption_centered=true)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_BUTTON,
            GComponentDef::specific_def =>
                array(
                    GCompButtonDef::caption => $caption,
                    GCompButtonDef::selected => $selected,
                    GCompButtonDef::caption_centered => $caption_centered,
                ),
        );
    }

    public static function get_scrollbar_def($geom, $margins,
        $scroll_pane_id, $vertical=true)
    {
        return array(
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_SCROLLBAR,
            GComponentDef::specific_def =>
                array(
                    GCompScrollbarDef::scroll_pane_id => $scroll_pane_id,
                    GCompScrollbarDef::vertical => $vertical,
                ),
        );
    }

    public static function get_view_position_def($x, $y, $id=null)
    {
        return array(
            GCompViewPositionDef::x => $x,
            GCompViewPositionDef::y => $y,
            GCompViewPositionDef::id => $id);
    }

    public static function get_panel_def($id,
        $geom, $margins, $children, $options = null)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::options => $options,
            GComponentDef::kind => GCOMPONENT_PANEL,
            GComponentDef::specific_def =>
                array(
                    GCompPanelDef::children => $children,
                ),
        );
    }

    public static function get_ppane_def($id, $geom,
        $children, $native_width=0, $native_height=0)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::kind => GCOMPONENT_PREPAINT_PANE,
            GComponentDef::specific_def =>
                array(
                    GCompPrepaintPaneDef::panel =>
                        array(GCompPanelDef::children => $children),
                    GCompPrepaintPaneDef::native_width => $native_width,
                    GCompPrepaintPaneDef::native_height => $native_height,
                ),
        );
    }

    public static function get_spane_def($id, $geom, $margins,
        $children, $view_width=0, $view_height=0, $view_position=null)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_SCROLL_PANE,
            GComponentDef::specific_def =>
                array(
                    GCompScrollPaneDef::panel =>
                        array(GCompPanelDef::children => $children),
                    GCompScrollPaneDef::view_width => $view_width,
                    GCompScrollPaneDef::view_height => $view_height,
                    GCompScrollPaneDef::view_position => $view_position,
                ),
        );
    }

    public static function get_vertical_spane_def($id, $geom, $margins,
        $children, $view_position=null)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_SCROLL_PANE,
            GComponentDef::specific_def =>
                array(
                    GCompScrollPaneDef::panel =>
                        array(GCompPanelDef::children => $children),
                    GCompScrollPaneDef::view_use_base_width => true,
                    GCompScrollPaneDef::view_position => $view_position,
                ),
        );
    }

    public static function get_horizontal_spane_def($id, $geom, $margins,
        $children, $view_position=null)
    {
        return array(
            GComponentDef::id => $id,
            GComponentDef::geom_def => $geom,
            GComponentDef::margins_def => $margins,
            GComponentDef::kind => GCOMPONENT_SCROLL_PANE,
            GComponentDef::specific_def =>
                array(
                    GCompScrollPaneDef::panel =>
                        array(GCompPanelDef::children => $children),
                    GCompScrollPaneDef::view_use_base_height => true,
                    GCompScrollPaneDef::view_position => $view_position,
                ),
        );
    }

    public static function get_window_def($comp_defs, $ui_state=null,
        $background_url=null,
        $background_color=null,
        $not_loaded_background_url=null,
        $async_loading_background=false,
        $playback_bg_alpha=-1,
        $fit_def=null,
        $small_state_text=null, $opaque_background=false)
    {
        return array(
            GCompWindowDef::background_color => $background_color,
            GCompWindowDef::background_url => $background_url,
            GCompWindowDef::async_loading_background => $async_loading_background,
            GCompWindowDef::comp_defs => $comp_defs,
            GCompWindowDef::ui_state => $ui_state,
            GCompWindowDef::not_loaded_background_url => $not_loaded_background_url,
            GCompWindowDef::playback_bg_alpha => $playback_bg_alpha,
            GCompWindowDef::background_fit_def => $fit_def,
            GCompWindowDef::small_state_text => $small_state_text,
            GCompWindowDef::opaque_background => $opaque_background,
        );
    }

    public static function get_image_fit_def($base_halign_ratio, $base_valign_ratio)
    {
        return array(
            ImageFitDef::base_halign_ratio => $base_halign_ratio,
            ImageFitDef::base_valign_ratio => $base_valign_ratio);
    }

    public static function get_comp_item_def($id, $caption, $icon_url=null)
    {
        return array(
            GCompItemDef::type => GCOMP_ITEM_REGULAR,
            GCompItemDef::id => $id,
            GCompItemDef::caption => $caption,
            GCompItemDef::icon_url => $icon_url);
    }

    public static function get_label_item_def($caption, $left_caption=null)
    {
        return array(
            GCompItemDef::type => GCOMP_ITEM_LABEL,
            GCompItemDef::left_caption => $left_caption,
            GCompItemDef::caption => $caption);
    }

    public static function get_button_item_def($id, $caption, $left_caption=null)
    {
        return array(
            GCompItemDef::type => GCOMP_ITEM_BUTTON,
            GCompItemDef::id => $id,
            GCompItemDef::left_caption => $left_caption,
            GCompItemDef::caption => $caption);
    }

    public static function get_gcomp_ui_state_def($items,
        $title=null, $bg_url=null, $poster_url=null, $view_params=null)
    {
        return array(
            GCompUiStateDef::items => $items,
            GCompUiStateDef::title => $title,
            GCompUiStateDef::bg_url => $bg_url,
            GCompUiStateDef::poster_url => $poster_url,
            GCompUiStateDef::view_params => $view_params,
        );
    }

    public static function set_focusable(&$def, $sel_margins=null,
        $background_url=null)
    {
        $def[GComponentDef::focusable_def] = array(
            GCompFocusableDef::sel_margins => $sel_margins,
            GCompFocusableDef::background_url => $background_url,
        );
    }

    public static function set_focus_var(&$def,
        $ref_id, $geom, $props=null)
    {
        $def[GComponentDef::focused_variant_def] = array(
            GCompVariantDef::ref_id => $ref_id,
            GCompVariantDef::geom_def => $geom,
            GCompVariantDef::props => $props,
        );
    }

    public static function add_extra_var(&$def,
        $ref_id, $geom, $props=null)
    {
        $def[GComponentDef::extra_variant_defs][] = array(
            GCompVariantDef::ref_id => $ref_id,
            GCompVariantDef::geom_def => $geom,
            GCompVariantDef::props => $props,
        );
    }

	public static function get_sticker_def($icon_url, $text = null, $color = null, $rect = null)
	{
		return
			array
			(
				PluginRegularSticker::icon_url => $icon_url,
				PluginRegularSticker::text => $text,
				PluginRegularSticker::color => $color,
				PluginRegularSticker::r => $rect,
			);
	}

    ////////////////////////////////////////////////////////////

    public static function get_change_def($id, $geom, $props=null,
        $view_position=null, $selected=null, $children=null,
        $transition=GCOMP_TRANSITION_DEFAULT)
    {
        return array(
            ChangeGCompDef::id => $id,
            ChangeGCompDef::geom_def => $geom,
            ChangeGCompDef::props => $props,
            ChangeGCompDef::view_position => $view_position,
            ChangeGCompDef::selected => $selected,
            ChangeGCompDef::children => $children,
            ChangeGCompDef::transition => $transition,
        );
    }
}

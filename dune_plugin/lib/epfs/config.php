<?php
// Config for embeddable plugin folders

class PaneParams
{
	const	dx							= 0;
	const	dy							= 0;
	const	width						= 1920;
	const	height						= 1080;
	const	info_dx						= 100;
	const	info_dy						= 40;
	const	info_width					= 800;
	const	info_height					= 640;
	const	vod_width					= 1280;
	const	vod_height					= 720;
	const	vod_bg_url					= '/bg.jpg';
	const	vod_mask_url				= '/mask.png';
	const	max_items_in_row			= 7;
	const	ch_num_font_color			= '#FFEFAA16';
	const	ch_num_font_size			= 64;
	const	ch_title_font_color			= '#FFEFAA16';
	const	ch_title_font_size			= 64;
	const	prog_title_font_color		= '#FFFFFFD0';
	const	prog_title_font_size		= 40;
	const	prog_item_font_color		= '#FFAFAFA0';
	const	prog_item_font_size			= 30;
	const	prog_item_height			= 40;
}

class RowsParams
{
	const	dx							= 0;
	const	width						= 1920;
	const	height						= 230;
	const	left_padding				= 90;
	const	inactive_left_padding		= 120;
	const	right_padding				= 120;
	const	hide_captions				= false;
	const	fade_enable					= true;
	const	fade_icon_mix_color			= 0;
	const	fade_icon_mix_alpha			= 170;
	const	lite_fade_icon_mix_alpha	= 128;
	const	fade_caption_color			= '#FF808080';
}

class TitleRowsParams
{
	const	width						= 1920;
	const	height						= 65;
	const	font_size					= 35;
	const	left_padding				= 115;
	const	fade_enabled				= true;
	const	fade_color					= '#FF606060';
	const	lite_fade_color				= '#FF808080';
	const	def_caption_color			= '#FFFFFFE0';
	const	fav_caption_color			= '#FF6C52CE';
}

class RowsItemsParams
{
	const	width						= 250;
	const	height						= 230;
	const	icon_width					= 230;
	const	icon_height					= 142;
    const	caption_dy					= 0;
    const	caption_max_num_lines		= 2;
    const	caption_line_spacing		= 0;
    const	def_caption_color			= '#FFAFAFA0';
    const	sel_caption_color			= '#FFFFFFE0';
    const	inactive_caption_color		= '#00000000';
    const	caption_font_size			= 28;
    const	icon_loading_url			= '/loading.png';
    const	icon_loading_failed_url		= '/unset.png';
    const	fav_sticker_icon_url		= '/star.png';
    const	fav_sticker_bg_color		= '#FF000000';
}

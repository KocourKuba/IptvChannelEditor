<?php

class PluginConfig
{
    const PluginVersion = '2.6.0';
    const PluginDate = '27.07.2021';
    const VOD_MOVIE_PAGE_SUPPORTED = true;
    const VOD_FAVORITES_SUPPORTED = true;
    const TV_FAVORITES_SUPPORTED = true;

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/iptv/{UID}/{ID}/index.m3u8';
    const CHANNEL_LIST_URL = 'edem_channel_list.xml';
    const EPG_URL_FORMAT = 'http://epg.ott-play.com/edem/epg/%d.json';
    const TVG_URL_FORMAT = 'http://www.teleguide.info/kanal%d_%s.html';
    const EPG_CACHE_DIR = '/tmp/edem_epg/';
    /** prefix to create cache file*/
    const EPG_CACHE_FILE = 'epg_channel_';
    const VOD_CATEGORIES_URL = 'http://online.dune-hd.com/demo2/vod_categories.pl';
    const MOVIE_LIST_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_list.pl?category_id=%s';
    const MOVIE_INFO_URL_FORMAT = 'http://online.dune-hd.com/demo2/movie_info.pl?movie_id=%s';

    const ALL_CHANNEL_GROUP_CAPTION = 'Все каналы';
    const ALL_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/all.png';

    const FAV_CHANNEL_GROUP_ID = '__favorites';
    const FAV_CHANNEL_GROUP_CAPTION = 'Избранное';
    const FAV_CHANNEL_GROUP_ICON_PATH = 'plugin_file://icons/fav.png';

    const FAV_MOVIES_CATEGORY_CAPTION = 'Избранное';
    const FAV_MOVIES_CATEGORY_ICON_PATH = 'plugin_file://icons/fav.png';

    ///////////////////////////////////////////////////////////////////////
    // Folder views.

    public static function GET_TV_GROUP_LIST_FOLDER_VIEWS()
    {
        return array(

            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::content_box_padding_left => 70,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }

    public static function GET_TV_CHANNEL_LIST_FOLDER_VIEWS()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 2,
                    ViewParams::num_rows => 10,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_LEFT,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::icon_dx => 10,
                    ViewItemParams::icon_dy => -5,
                    ViewItemParams::icon_width => 84,
                    ViewItemParams::icon_height => 48,
                    ViewItemParams::item_caption_width => 485,
                    ViewItemParams::item_caption_font_size => FONT_SIZE_SMALL,
                    ViewItemParams::icon_path => 'plugin_file://icons/channel_unset.png',
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => 'plugin_file://icons/channel_unset.png'
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => true,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                    ViewItemParams::icon_path => 'plugin_file://icons/channel_unset.png',
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 4,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                    ViewItemParams::icon_path => 'plugin_file://icons/channel_unset.png',
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 3,
                    ViewParams::num_rows => 3,
                    ViewParams::background_path => 'plugin_file://icons/bg.jpg',
                    ViewParams::background_order => 0,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                    ViewItemParams::icon_path => 'plugin_file://icons/channel_unset.png',
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            )
        );
    }

    public static function GET_VOD_MOVIE_LIST_FOLDER_VIEWS()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => true,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 6,
                    ViewParams::num_rows => 3,
                    ViewParams::icon_selection_box_width => 150,
                    ViewParams::icon_selection_box_height => 222,
                    ViewParams::paint_details => true,
                    ViewParams::zoom_detailed_icon => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_padding_top => 0,
                    ViewItemParams::item_padding_bottom => 0,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array
                (
                    ViewItemParams::icon_path => 'plugin_file://icons/mov_unset.png',
                    ViewItemParams::item_detailed_icon_path => 'missing://',
                ),
            ));
    }

    public static function GET_VOD_CATEGORY_LIST_FOLDER_VIEWS()
    {
        return array(
            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 5,
                    ViewParams::num_rows => 4,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => true,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.0,
                    ViewItemParams::icon_sel_scale_factor => 1.2,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),

            array
            (
                PluginRegularFolderView::async_icon_loading => false,

                PluginRegularFolderView::view_params => array
                (
                    ViewParams::num_cols => 4,
                    ViewParams::num_rows => 3,
                    ViewParams::paint_details => false,
                    ViewParams::paint_sandwich => true,
                    ViewParams::sandwich_base => 'gui_skin://special_icons/sandwich_base.aai',
                    ViewParams::sandwich_mask => 'cut_icon://{name=sandwich_mask}',
                    ViewParams::sandwich_cover => 'cut_icon://{name=sandwich_cover}',
                    ViewParams::sandwich_width => 245,
                    ViewParams::sandwich_height => 140,
                    ViewParams::sandwich_icon_upscale_enabled => true,
                    ViewParams::sandwich_icon_keep_aspect_ratio => false,
                ),

                PluginRegularFolderView::base_view_item_params => array
                (
                    ViewItemParams::item_paint_icon => true,
                    ViewItemParams::item_layout => HALIGN_CENTER,
                    ViewItemParams::icon_valign => VALIGN_CENTER,
                    ViewItemParams::item_paint_caption => false,
                    ViewItemParams::icon_scale_factor => 1.25,
                    ViewItemParams::icon_sel_scale_factor => 1.5,
                ),

                PluginRegularFolderView::not_loaded_view_item_params => array(),
            ),
        );
    }
}

?>

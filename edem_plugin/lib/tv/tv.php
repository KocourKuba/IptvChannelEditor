<?php
///////////////////////////////////////////////////////////////////////////

interface Tv
{
    /** Return Array<Channel>*/
    public function get_channels();

    /** Return Array<Group>*/
    public function get_groups();

    public function unload_channels();

    public function ensure_channels_loaded(&$plugin_cookies);

    public function get_all_channel_group_id();

    public function get_fav_icon_url();

    public function get_tv_info(MediaURL $media_url, &$plugin_cookies);

    public function get_tv_stream_url($playback_url, &$plugin_cookies);

    public function get_tv_playback_url($channel_id, $archive_ts, $protect_code, &$plugin_cookies);

    public function get_day_epg($channel_id, $day_start_ts, &$plugin_cookies);

    public function change_tv_favorites($fav_op_type, $channel_id, &$plugin_cookies);

    public function get_archive(MediaURL $media_url);

    /** Hook.*/
    public function folder_entered(MediaURL $media_url, &$plugin_cookies);

    /** Hook for adding special group items.*/
    public function add_special_groups(&$items);

    /** Get id's stored in the cookie.*/
    public function get_fav_channel_ids($plugin_cookies);

    /** Put id's in the cookie.*/
    public function set_fav_channel_ids($plugin_cookies, $ids);
}

///////////////////////////////////////////////////////////////////////////
?>

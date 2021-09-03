<?php

interface IChannel
{
    // unique id for channel. typically hash
    public function get_id();

    // channel title
    public function get_title();

    // icon uri
    public function get_icon_url();

    public function get_groups(); // Array<Group>

    // internal number
    public function get_number();

    // is channel support archive playback
    public function has_archive();

    // is protected (adult)
    public function is_protected();

    // primary EPG source
    public function get_epg_id();

    // secondary EPG source
    public function get_tvg_id();

    public function get_past_epg_days();

    public function get_future_epg_days();

    public function get_archive_past_sec();

    public function get_archive_delay_sec();

    public function get_buffering_ms();

    // timeshift for epg to this channel
    public function get_timeshift_hours();

    // streaming url
    public function get_streaming_url();
}

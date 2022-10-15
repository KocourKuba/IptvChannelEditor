<?php

interface Channel
{
    /**
     * unique id for channel. typically hash
     * @return string
     */
    public function get_id();

    /**
     * channel title
     * @return string
     */
    public function get_title();

    /**
     * icon uri
     * @return string
     */
    public function get_icon_url();

    /**
     * @return array|Group[]
     */
    public function get_groups();

    /**
     * internal number
     * @return int
     */
    public function get_number();

    /**
     * is channel support archive playback
     * @return bool
     */
    public function has_archive();

    /**
     * is protected (adult)
     * @return bool
     */
    public function is_protected();

    /**
     * primary EPG source
     * @return string
     */
    public function get_epg_id();

    /**
     * secondary EPG source
     * @return string
     */
    public function get_tvg_id();

    /**
     * @return int
     */
    public function get_past_epg_days();

    /**
     * @return int
     */
    public function get_future_epg_days();

    /**
     * @return int
     */
    public function get_archive_past_sec();

    /**
     * @return int
     */
    public function get_archive_delay_sec();

    /**
     * timeshift for epg to this channel
     * @return int
     */
    public function get_timeshift_hours();

    /**
     * streaming url
     * @return string
     */
    public function get_streaming_url();

    /**
     * custom streaming url archive template
     * @return string
     */
    public function get_custom_arc_template();

    /**
     * additional parameters
     * @return array
     */
    public function get_ext_params();
}

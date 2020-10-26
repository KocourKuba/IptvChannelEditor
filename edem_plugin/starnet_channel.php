<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/tv/default_channel.php';

///////////////////////////////////////////////////////////////////////////

class DemoChannel extends DefaultChannel
{
    private $has_archive;
    private $number;
    private $past_epg_days;
    private $future_epg_days;
    private $is_protected;
    private $timeshift_hours;
    private $buf_time;

    ///////////////////////////////////////////////////////////////////////

    public function __construct(
        $id, $title, $icon_url, $has_archive, $streaming_url, $number, $past_epg_days, $future_epg_days, $is_protected, $timeshift_hours, $buf_time)
    {
//        hd_print($streaming_url);
        parent::__construct($id, $title, $icon_url, $streaming_url);

        $this->has_archive = $has_archive;
        $this->number = $number;
        $this->past_epg_days = $past_epg_days;
        $this->future_epg_days = $future_epg_days;
        $this->is_protected = $is_protected;
        $this->timeshift_hours = $timeshift_hours;
        $this->buf_time = $buf_time;
    }

    ///////////////////////////////////////////////////////////////////////

    public function is_protected()
    {
        return $this->is_protected;
    }

    public function has_archive()
    {
        return $this->has_archive;
    }

    public function get_timeshift_hours()
    {
        return $this->timeshift_hours;
    }

    public function get_archive_past_sec()
    {
        return 4 * 86400;
    }

    public function get_archive_delay_sec()
    {
        return 5 * 60;
    }

    public function get_number()
    {
        return $this->number;
    }

    public function get_past_epg_days()
    {
        return $this->past_epg_days;
    }

    public function get_future_epg_days()
    {
        return $this->future_epg_days;
    }

    public function get_buffering_ms()
    {
        return $this->buf_time;
    }
}

///////////////////////////////////////////////////////////////////////////
?>

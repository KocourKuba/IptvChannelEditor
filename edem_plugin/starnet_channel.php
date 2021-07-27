<?php
///////////////////////////////////////////////////////////////////////////

require_once 'lib/tv/default_channel.php';

///////////////////////////////////////////////////////////////////////////

class DemoChannel extends DefaultChannel
{
    private $has_archive;
    private $number;
    private $epg_id;
    private $tvg_id;
    private $is_protected;
    private $timeshift_hours;

    ///////////////////////////////////////////////////////////////////////

    public function __construct(
        $id, $title, $icon_url, $has_archive, $streaming_url, $number, $tvg_id, $epg_id, $is_protected, $timeshift_hours)
    {
        parent::__construct($id, $title, $icon_url, $streaming_url);

        $this->has_archive = $has_archive;
        $this->number = $number;
        $this->tvg_id = $tvg_id;
        $this->epg_id = $epg_id;
        $this->is_protected = $is_protected;
        $this->timeshift_hours = $timeshift_hours;
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
        return 7 * 86400;
    }

    public function get_archive_delay_sec()
    {
        return 5 * 60;
    }

    public function get_number()
    {
        return $this->number;
    }

    public function get_tvg_id()
    {
        return $this->tvg_id;
    }

    public function get_epg_id()
    {
        return $this->epg_id;
    }
}

///////////////////////////////////////////////////////////////////////////
?>

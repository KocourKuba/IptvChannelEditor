<?php
///////////////////////////////////////////////////////////////////////////

require_once 'channel.php';

///////////////////////////////////////////////////////////////////////////

class DefaultChannel implements IChannel
{
    protected $_id;
    protected $_channel_id;
    protected $_title;
    protected $_icon_url;
    protected $_streaming_url;
    protected $_groups; // Array<Group>
    protected $_number;
    protected $_has_archive;
    protected $_is_protected;
    protected $_epg_id;
    protected $_tvg_id;
    protected $_timeshift_hours;

    public function __construct($id, $channel_id, $title, $icon_url, $streaming_url, $has_archive, $number, $tvg_id, $epg_id, $is_protected, $timeshift_hours)
    {
        $this->_id = $id;
        $this->_channel_id = $channel_id;
        $this->_title = $title;
        $this->_icon_url = $icon_url;
        $this->_streaming_url = $streaming_url;
        $this->_groups = array();
        $this->_has_archive = $has_archive;
        $this->_number = $number;
        $this->_tvg_id = $tvg_id;
        $this->_epg_id = $epg_id;
        $this->_is_protected = $is_protected;
        $this->_timeshift_hours = $timeshift_hours;
    }

    public function get_id()
    {
        return $this->_id;
    }

    public function get_channel_id()
    {
        return $this->_channel_id;
    }

    public function get_title()
    {
        return $this->_title;
    }

    public function get_icon_url()
    {
        return $this->_icon_url;
    }

    public function get_groups()
    {
        return $this->_groups;
    }

    public function get_number()
    {
        return $this->_number;
    }

    public function is_protected()
    {
        return $this->_is_protected;
    }

    public function has_archive()
    {
        return $this->_has_archive;
    }

    public function get_timeshift_hours()
    {
        return $this->_timeshift_hours;
    }

    public function get_tvg_id()
    {
        return $this->_tvg_id;
    }

    public function get_epg_id()
    {
        return $this->_epg_id;
    }

    public function get_past_epg_days()
    {
        return 7;
    }

    public function get_future_epg_days()
    {
        return 7;
    }

    public function get_archive_past_sec()
    {
        return 7 * 86400;
    }

    public function get_archive_delay_sec()
    {
        return 5 * 60;
    }

    public function get_buffering_ms()
    {
        return 0;
    }

    public function get_streaming_url()
    {
        return $this->_streaming_url;
    }

    ///////////////////////////////////////////////////////////////////////

    public function add_group($group)
    {
        $this->_groups[] = $group;
    }
}

///////////////////////////////////////////////////////////////////////////
?>

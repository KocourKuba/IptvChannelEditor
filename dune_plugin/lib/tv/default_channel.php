<?php
require_once 'channel.php';

class DefaultChannel implements IChannel
{
    protected $_id;
    protected $_channel_id;
    protected $_title;
    protected $_icon_url;
    protected $_streaming_url;
    protected $_groups; // Array<Group>
    protected $_number;
    protected $_archive;
    protected $_is_protected;
    protected $_epg_id;
    protected $_tvg_id;
    protected $_timeshift_hours;
    protected $_ext_params;

    public function __construct($id, $channel_id, $title, $icon_url, $streaming_url, $_archive, $number, $epg_id, $tvg_id, $is_protected, $timeshift_hours, $ext_params)
    {
        $this->_id = $id;
        $this->_channel_id = $channel_id;
        $this->_title = $title;
        $this->_icon_url = $icon_url;
        $this->_streaming_url = $streaming_url;
        $this->_groups = array();
        $this->_archive = ($_archive > 0) ? $_archive : 0;
        $this->_number = $number;
        $this->_epg_id = $epg_id;
        $this->_tvg_id = $tvg_id;
        $this->_is_protected = $is_protected;
        $this->_timeshift_hours = $timeshift_hours;
        $this->_ext_params = $ext_params;
    }

    /**
     * get id (hash)
     * @return string
     */
    public function get_id()
    {
        return $this->_id;
    }

    /**
     * get channel id
     * @return string
     */
    public function get_channel_id()
    {
        return $this->_channel_id;
    }

    /**
     * get channel title
     * @return string
     */
    public function get_title()
    {
        return $this->_title;
    }

    /**
     * @return string
     */
    public function get_icon_url()
    {
        return $this->_icon_url;
    }

    /**
     * get groups array
     * @return array
     */
    public function get_groups()
    {
        return $this->_groups;
    }

    /**
     * get channel number
     * @return mixed
     */
    public function get_number()
    {
        return $this->_number;
    }

    /**
     * is channel protected (adult)
     * @return bool
     */
    public function is_protected()
    {
        return $this->_is_protected;
    }

    /**
     * is channel has archive playback
     * @return bool
     */
    public function has_archive()
    {
        return $this->_archive > 0;
    }

    /**
     * get timeshift
     * @return mixed
     */
    public function get_timeshift_hours()
    {
        return $this->_timeshift_hours;
    }

    /**
     * get EPG id (secondary)
     * @return string
     */
    public function get_tvg_id()
    {
        return $this->_tvg_id;
    }

    /**
     * get EPG id (primary)
     * @return string
     */
    public function get_epg_id()
    {
        return $this->_epg_id;
    }

    /**
     * how many epg reads from the past
     * @return int
     */
    public function get_past_epg_days()
    {
        return $this->_archive > 1 ? $this->_archive : 7;
    }

    /**
     * how many epg reads forward
     * @return int
     */
    public function get_future_epg_days()
    {
        return 7;
    }

    /**
     * how many second playback from the past
     * @return float|int
     */
    public function get_archive_past_sec()
    {
        return $this->_archive * 86400;
    }

    /**
     * @return float|int
     */
    public function get_archive_delay_sec()
    {
        return 7 * 60;
    }

    /**
     * get playback url
     * @return string
     */
    public function get_streaming_url()
    {
        return $this->_streaming_url;
    }

    /**
     * get additional parameters (filled from provider m3u8)
     * @return array
     */
    public function get_ext_params()
    {
        return $this->_ext_params;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * add group
     * @param $group
     */
    public function add_group($group)
    {
        $this->_groups[] = $group;
    }
}

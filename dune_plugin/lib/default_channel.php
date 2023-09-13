<?php
require_once 'channel.php';
require_once 'json_serializer.php';

class Default_Channel extends Json_Serializer implements Channel
{
    /**
     * @var string
     */
    protected $_id;

    /**
     * @var string
     */
    protected $_channel_id;

    /**
     * @var string
     */
    protected $_title;

    /**
     * @var string
     */
    protected $_desc;

    /**
     * @var string
     */
    protected $_icon_url;

    /**
     * @var string
     */
    protected $_streaming_url;

    /**
     * @var string
     */
    protected $_custom_arc_template;

    /**
     * @var string
     */
    protected $_streaming_url_type;

    /**
     * @var string
     */
    protected $_custom_arc_url_type;

    /**
     * @var array[Groups]
     */
    protected $_groups;

    /**
     * @var int
     */
    protected $_number;

    /**
     * @var int
     */
    protected $_archive;

    /**
     * @var bool
     */
    protected $_protected;

    /**
     * @var bool
     */
    protected $_disabled;

    /**
     * @var array first epg
     */
    protected $_epg_ids;

    /**
     * @var int
     */
    protected $_timeshift_hours;

    /**
     * @var array|null
     */
    protected $_ext_params;

    /**
     * @param string $id
     * @param string $channel_id
     * @param string $title
     * @param string $icon_url
     * @param string $streaming_url
     * @param string $custom_arc_template
     * @param string $streaming_url_type
     * @param string $custom_arc_url_type
     * @param int $archive
     * @param int $number
     * @param array $epg_ids
     * @param bool $protected
     * @param int $timeshift_hours
     * @param array|null $ext_params
     */
    public function __construct($id, $channel_id, $title, $icon_url,
                                $streaming_url, $custom_arc_template,
                                $streaming_url_type, $custom_arc_url_type,
                                $archive, $number, $epg_ids,
                                $protected, $timeshift_hours, $ext_params)
    {
        $this->_disabled = false;

        $this->_id = $id;
        $this->_channel_id = $channel_id;
        $this->_title = $title;
        $this->_icon_url = $icon_url;
        $this->_streaming_url = $streaming_url;
        $this->_custom_arc_template = $custom_arc_template;
        $this->_streaming_url_type = $streaming_url_type;
        $this->_custom_arc_url_type = $custom_arc_url_type;
        $this->_groups = array();
        $this->_archive = ($archive > 0) ? $archive : 0;
        $this->_number = $number;
        $this->_epg_ids = $epg_ids;
        $this->_protected = $protected;
        $this->_timeshift_hours = $timeshift_hours;
        $this->_ext_params = $ext_params;
    }

    /**
     * @inheritDoc
     */
    public function get_id()
    {
        return $this->_id;
    }

    /**
     * get id (hash)
     * @return string
     */
    public function get_channel_id()
    {
        return $this->_channel_id;
    }

    /**
     * @inheritDoc
     */
    public function get_title()
    {
        return $this->_title;
    }

    /**
     * @inheritDoc
     */
    public function get_desc()
    {
        return $this->_desc;
    }

    /**
     * @inheritDoc
     */
    public function set_desc($desc)
    {
        $this->_desc = $desc;
    }

    /**
     * @inheritDoc
     */
    public function get_icon_url()
    {
        return $this->_icon_url;
    }

    /**
     * @inheritDoc
     */
    public function get_groups()
    {
        return $this->_groups;
    }

    /**
     * @inheritDoc
     */
    public function get_number()
    {
        return $this->_number;
    }

    /**
     * @inheritDoc
     */
    public function is_protected()
    {
        return $this->_protected;
    }

    /**
     * @inheritDoc
     */
    public function is_disabled()
    {
        return $this->_disabled;
    }

    /**
     * @inheritDoc
     */
    public function set_disabled($disabled)
    {
        $this->_disabled = $disabled;
    }

    /**
     * @inheritDoc
     */
    public function get_archive()
    {
        return $this->_archive;
    }

    /**
     * @inheritDoc
     */
    public function get_timeshift_hours()
    {
        return $this->_timeshift_hours;
    }

    /**
     * @inheritDoc
     */
    public function get_epg_ids()
    {
        return $this->_epg_ids;
    }

    /**
     * @inheritDoc
     */
    public function get_past_epg_days()
    {
        return $this->_archive > 1 ? $this->_archive : 7;
    }

    /**
     * @inheritDoc
     */
    public function get_future_epg_days()
    {
        return 7;
    }

    /**
     * @inheritDoc
     */
    public function get_archive_past_sec()
    {
        return $this->_archive * 86400;
    }

    /**
     * @inheritDoc
     */
    public function get_archive_delay_sec()
    {
        return 60;
    }

    /**
     * @inheritDoc
     */
    public function get_custom_url()
    {
        return $this->_streaming_url;
    }

    /**
     * @inheritDoc
     */
    public function get_custom_archive_template()
    {
        return $this->_custom_arc_template;
    }

    /**
     * @inheritDoc
     */
    public function get_custom_url_type()
    {
        return $this->_streaming_url_type;
    }

    /**
     * @inheritDoc
     */
    public function get_custom_archive_url_type()
    {
        return $this->_custom_arc_url_type;
    }

    /**
     * @inheritDoc
     */
    public function get_ext_params()
    {
        return $this->_ext_params;
    }

    ///////////////////////////////////////////////////////////////////////
    ///

    /**
     * set additional parameters (filled from provider m3u8)
     * @param array $ext_params
     */
    public function set_ext_params($ext_params)
    {
        $this->_ext_params = $ext_params;
    }

    /**
     * set additional parameters (filled from provider m3u8)
     * @param $param
     * @param $value
     */
    public function set_ext_param($param, $value)
    {
        $this->_ext_params[$param] = $value;
    }

    /**
     * add group
     * @param Group $group
     */
    public function add_group(Group $group)
    {
        $this->_groups[] = $group;
    }
}

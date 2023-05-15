<?php

class dynamic_config
{
    // info
    public $plugin_info;

    // features constants
    private $features = array();
    private $stream_params = array();
    private $epg_parser_params = array();
    private $vod_templates = array();
    private $servers = array();
    private $devices = array();
    private $qualities = array();
    private $profiles = array();

    /**
     * load configuration
     * @return void
     */
    public function init_defaults()
    {
        hd_print(__METHOD__);
        $this->features[Plugin_Constants::TV_FAVORITES_SUPPORTED] = true; // always true
        $this->features[Plugin_Constants::BALANCE_SUPPORTED] = false; // account support account info requests
        $this->features[Plugin_Constants::VOD_QUALITY_SUPPORTED] = false; // currently supported only in edem
        $this->features[Plugin_Constants::VOD_FILTER_SUPPORTED] = false; // filter list screen

        $this->features[Plugin_Constants::ACCESS_TYPE] = Plugin_Constants::ACCOUNT_UNKNOWN;
        $this->features[Plugin_Constants::SQUARE_ICONS] = false;
        $this->features[Plugin_Constants::USER_AGENT] = "DuneHD/1.0";
        $this->features[Plugin_Constants::VOD_SUPPORTED] = false;
        $this->features[Plugin_Constants::VOD_M3U] = false;

        $this->features[Plugin_Constants::PLAYLIST_TEMPLATE_INDEX] = 0;


        // load defaults
        $default_streams = array();
        $default_streams[Stream_Params::URL_TEMPLATE] = '';
        $default_streams[Stream_Params::URL_ARC_TEMPLATE] = '{LIVE_URL}?utc={START}&lutc={NOW}';
        $default_streams[Stream_Params::URL_CUSTOM_ARC_TEMPLATE] = '{LIVE_URL}?utc={START}&lutc={NOW}';
        $default_streams[Stream_Params::CU_TYPE] = 'shift';
        $default_streams[Stream_Params::CU_DURATION] = 10800;
        $default_streams[Stream_Params::DUNE_PARAMS] = '';
        $this->set_stream_params(Plugin_Constants::HLS, $default_streams);

        $default_streams[Stream_Params::CU_TYPE] = 'flussonic';
        $default_streams[Stream_Params::URL_ARC_TEMPLATE] = '';
        $default_streams[Stream_Params::DUNE_PARAMS] = 'buffering_ms:{BUFFERING}';
        $this->set_stream_params(Plugin_Constants::MPEG, $default_streams);

        $default_parser = array();
        $default_parser[Plugin_Constants::EPG_PARSER] = 'json';
        $default_parser[Epg_Params::EPG_URL] = '';
        $default_parser[Epg_Params::EPG_ROOT] = 'epg_data';
        $default_parser[Epg_Params::EPG_START] = 'time';
        $default_parser[Epg_Params::EPG_END] = 'time_to';
        $default_parser[Epg_Params::EPG_NAME] = 'name';
        $default_parser[Epg_Params::EPG_DESC] = 'descr';
        $default_parser[Epg_Params::EPG_DATE_FORMAT] = '';
        $default_parser[Epg_Params::EPG_USE_DURATION] = false;
        $default_parser[Epg_Params::EPG_TIME_FORMAT] = '';
        $default_parser[Epg_Params::EPG_TIMEZONE] = 0;
        $default_parser[Plugin_Constants::EPG_ID_MAPPER] = array();

        $this->set_epg_params(Plugin_Constants::EPG_FIRST, $default_parser);
        $this->set_epg_params(Plugin_Constants::EPG_SECOND, $default_parser);
    }

    /**
     * load configuration
     * @return void
     */
    public function load_config()
    {
        hd_print(__METHOD__);
        $settings = HD::parse_json_file(get_install_path('config.json'), true);

        $this->set_feature(Plugin_Constants::ACCESS_TYPE, $settings[Plugin_Constants::ACCESS_TYPE]);
        $this->set_feature(Plugin_Constants::SQUARE_ICONS, $settings[Plugin_Constants::SQUARE_ICONS]);
        $this->set_feature(Plugin_Constants::PLAYLIST_TEMPLATES, $settings[Plugin_Constants::PLAYLIST_TEMPLATES]);
        $this->set_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX, $settings[Plugin_Constants::PLAYLIST_TEMPLATE_INDEX]);
        $this->set_feature(Plugin_Constants::VOD_SUPPORTED, $settings[Plugin_Constants::VOD_SUPPORTED]);
        $this->set_feature(Plugin_Constants::VOD_M3U, $settings[Plugin_Constants::VOD_M3U]);
        $this->set_feature(Plugin_Constants::VOD_TEMPLATES, $settings[Plugin_Constants::VOD_TEMPLATES]);

        HD::set_dune_user_agent($settings[Plugin_Constants::USER_AGENT]);
        HD::set_plugin_dev_code($settings[Plugin_Constants::DEV_CODE]);

        foreach ($settings[Plugin_Constants::STREAMS_CONFIG] as $config) {
            $param_idx = $config[Stream_Params::STREAM_TYPE];
            $params = $this->get_stream_params($param_idx);
            $this->set_stream_params($param_idx, array_merge($params, $config));
            //hd_print(__METHOD__ . ": stream config: $param_idx");
            //foreach($this->get_stream_params($param_idx) as $key=>$value) hd_print("$key: $value");
        }

        foreach ($settings[Plugin_Constants::EPG_PARAMS] as $epg) {
            $param_idx = $epg[Epg_Params::EPG_PARAM];
            $params = $this->get_epg_params($param_idx);
            $this->set_epg_params($param_idx, array_merge($params, $epg));
            //hd_print(__METHOD__ . ": epg_param: $param_idx");
            //foreach($epg as $key=>$value) hd_print("$key: $value");
        }

        $servers = array();
        foreach ($settings[Plugin_Constants::SERVERS_LIST] as $pair) {
            $servers[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_servers($servers);

        $devices = array();
        foreach ($settings[Plugin_Constants::DEVICES_LIST] as $pair) {
            $devices[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_devices($devices);

        $qualities = array();
        foreach ($settings[Plugin_Constants::QUALITIES_LIST] as $pair) {
            $qualities[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_qualities($qualities);

        $profiles = array();
        foreach ($settings[Plugin_Constants::PROFILES_LIST] as $pair) {
            $profiles[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_profiles($profiles);

        //HD::print_array($this->features);
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_feature($type)
    {
        return isset($this->features[$type]) ? $this->features[$type] : null;
    }

    /**
     * @param string $type
     * @param mixed $val
     */
    public function set_feature($type, $val)
    {
        $this->features[$type] = $val;
    }

    /**
     * @param string $type
     * @return array
     */
    public function get_stream_params($type)
    {
        return isset($this->stream_params[$type]) ? $this->stream_params[$type] : array();
    }

    /**
     * @param string $type
     * @param array $val
     */
    public function set_stream_params($type, $val)
    {
        $this->stream_params[$type] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @return mixed
     */
    public function get_stream_param($type, $param)
    {
        return $this->stream_params[$type][$param];
    }

    /**
     * @param string $type
     * @param string $param
     * @param mixed $val
     */
    public function set_stream_param($type, $param, $val)
    {
        $this->stream_params[$type][$param] = $val;
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_epg_params($type)
    {
        return $this->epg_parser_params[$type];
    }

    /**
     * @param string $type
     * @param array $val
     */
    public function set_epg_params($type, $val)
    {
        $this->epg_parser_params[$type] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @param mixed $val
     */
    public function set_epg_param($type, $param, $val)
    {
        $this->epg_parser_params[$type][$param] = $val;
    }

    /**
     * @param string $type
     * @param string $param
     * @return mixed
     */
    public function get_epg_param($type, $param)
    {
        return $this->epg_parser_params[$type][$param];
    }

    /**
     * @return array
     */
    public function get_servers($plugin_cookies)
    {
        return $this->servers;
    }

    /**
     * @param array $val
     */
    public function set_servers($val)
    {
        $this->servers = $val;
    }

    /**
     * @return array
     */
    public function get_devices($plugin_cookies)
    {
        return $this->devices;
    }

    /**
     * @param array $val
     */
    public function set_devices($val)
    {
        $this->devices = $val;
    }

    /**
     * @return array
     */
    public function get_qualities($plugin_cookies)
    {
        return $this->qualities;
    }

    /**
     * @param array $val
     */
    public function set_qualities($val)
    {
        $this->qualities = $val;
    }

    /**
     * @return array
     */
    public function get_profiles($plugin_cookies)
    {
        return $this->profiles;
    }

    /**
     * @param array $val
     */
    public function set_profiles($val)
    {
        $this->profiles = $val;
    }
}

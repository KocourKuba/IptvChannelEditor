<?php

class dynamic_config
{
    protected $PluginShortName;

    // features constants
    private $features = array();
    private $stream_params = array();
    private $epg_parser_params = array();
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
        hd_print("Init defaults");
        $this->features[Plugin_Constants::TV_FAVORITES_SUPPORTED] = true; // always true
        $this->features[Plugin_Constants::BALANCE_SUPPORTED] = false; // account support account info requests
        $this->features[Plugin_Constants::VOD_LAZY_LOAD] = false; // all movies loaded as one file or require delayed request to collect movies
        $this->features[Plugin_Constants::VOD_QUALITY_SUPPORTED] = false; // currently supported only in edem
        $this->features[Plugin_Constants::VOD_FILTER_SUPPORTED] = false; // filter list screen

        $this->features[Plugin_Constants::ACCESS_TYPE] = Plugin_Constants::ACCOUNT_UNKNOWN;
        $this->features[Plugin_Constants::SQUARE_ICONS] = false;
        $this->features[Plugin_Constants::PLAYLIST_TEMPLATE] = '';
        $this->features[Plugin_Constants::URI_PARSE_PATTERN] = '';
        $this->features[Plugin_Constants::VOD_SUPPORTED] = false;
        $this->features[Plugin_Constants::VOD_M3U] = false;
        $this->features[Plugin_Constants::VOD_PLAYLIST_URL] = '';
        $this->features[Plugin_Constants::VOD_PARSE_PATTERN] = '';

        // load defaults
        $default_streams = array();
        $default_streams[Stream_Params::CU_TYPE] = 'shift';
        $default_streams[Stream_Params::URL_TEMPLATE] = '';
        $default_streams[Stream_Params::URL_ARC_TEMPLATE] = '{CU_SUBST}={START}&lutc={NOW}';
        $default_streams[Stream_Params::URL_CUSTOM_ARC_TEMPLATE] = '{CU_SUBST}={START}&lutc={NOW}';
        $default_streams[Stream_Params::CU_SUBST] = 'utc';
        $default_streams[Stream_Params::CU_DURATION] = 10800;
        $this->set_stream_params(HLS, $default_streams);

        $default_streams[Stream_Params::CU_TYPE] = 'flussonic';
        $default_streams[Stream_Params::CU_SUBST] = 'archive';
        $default_streams[Stream_Params::URL_ARC_TEMPLATE] = '';
        $this->set_stream_params(MPEG, $default_streams);

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
        $settings = HD::parse_json_file(get_install_path('config.json'), true);
        hd_print("Load plugin settings");

        $this->PluginShortName = $settings[Plugin_Constants::SHORT_NAME];
        $this->set_feature(Plugin_Constants::ACCESS_TYPE, $settings[Plugin_Constants::ACCESS_TYPE]);
        $this->set_feature(Plugin_Constants::SQUARE_ICONS, $settings[Plugin_Constants::SQUARE_ICONS]);
        $this->set_feature(Plugin_Constants::PLAYLIST_TEMPLATE, $settings[Plugin_Constants::PLAYLIST_TEMPLATE]);
        $this->set_feature(Plugin_Constants::URI_PARSE_PATTERN, $settings[Plugin_Constants::URI_PARSE_PATTERN]);
        $this->set_feature(Plugin_Constants::TAG_ID_MATCH, $settings[Plugin_Constants::TAG_ID_MATCH]);
        $this->set_feature(Plugin_Constants::VOD_SUPPORTED, $settings[Plugin_Constants::VOD_SUPPORTED]);
        $this->set_feature(Plugin_Constants::VOD_M3U, $settings[Plugin_Constants::VOD_M3U]);
        $this->set_feature(Plugin_Constants::VOD_PLAYLIST_URL, $settings[Plugin_Constants::VOD_PLAYLIST_URL]);
        $this->set_feature(Plugin_Constants::VOD_PARSE_PATTERN, $settings[Plugin_Constants::VOD_PARSE_PATTERN]);

        foreach ($settings[Plugin_Constants::STREAMS_CONFIG] as $config)
        {
            $param_idx = $config[Stream_Params::STREAM_TYPE];
            $params = $this->get_stream_params($param_idx);
            $this->set_stream_params($param_idx, array_merge($params, $config));
            //hd_print("stream_config: $param_idx");
            //foreach($this->get_stream_params($param_idx) as $key=>$value) hd_print("$key: $value");
        }

        foreach ($settings[Plugin_Constants::EPG_PARAMS] as $epg)
        {
            $param_idx = $epg[Epg_Params::EPG_PARAM];
            $params = $this->get_epg_params($param_idx);
            $this->set_epg_params($param_idx, array_merge($params, $epg));
            //hd_print("epg_param: $param_idx");
            //foreach($epg as $key=>$value) hd_print("$key: $value");
        }

        $servers = array();
        foreach ($settings[Plugin_Constants::SERVERS_LIST] as $pair)
        {
            $servers[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_servers($servers);

        $devices = array();
        foreach ($settings[Plugin_Constants::DEVICES_LIST] as $pair)
        {
            $devices[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_devices($devices);

        $qualities = array();
        foreach ($settings[Plugin_Constants::QUALITIES_LIST] as $pair)
        {
            $qualities[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_qualities($qualities);

        $profiles = array();
        foreach ($settings[Plugin_Constants::PROFILES_LIST] as $pair)
        {
            $profiles[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_profiles($profiles);
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_feature($type)
    {
        return empty($this->features[$type]) ? null : $this->features[$type];
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
        return empty($this->stream_params[$type]) ? array() : $this->stream_params[$type];
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
     * @return string
     */
    public function get_server_name($plugin_cookies)
    {
        $servers = $this->get_servers($plugin_cookies);
        return $servers[$this->get_server_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server_id($plugin_cookies)
    {
        $servers = $this->get_servers($plugin_cookies);
        reset($servers);
        $first = key($servers);
        return isset($plugin_cookies->server, $servers[$plugin_cookies->server]) ? $plugin_cookies->server : $first;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
        $plugin_cookies->server = $server;
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
     * @return string
     */
    public function get_device_name($plugin_cookies)
    {
        $devices = $this->get_devices($plugin_cookies);
        return $devices[$this->get_device_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_device_id($plugin_cookies)
    {
        $devices = $this->get_devices($plugin_cookies);
        //reset($devices);
        $first = key($devices);
        return isset($plugin_cookies->device, $devices[$plugin_cookies->device]) ? $plugin_cookies->device : $first;
    }

    /**
     * @param $device
     * @param $plugin_cookies
     */
    public function set_device_id($device, $plugin_cookies)
    {
        $plugin_cookies->device = $device;
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
     * @return string
     */
    public function get_quality_name($plugin_cookies)
    {
        $qualities = $this->get_qualities($plugin_cookies);
        return $qualities[$this->get_quality_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_quality_id($plugin_cookies)
    {
        $quality = $this->get_qualities($plugin_cookies);
        reset($quality);
        $first = key($quality);
        return isset($plugin_cookies->quality, $quality[$plugin_cookies->quality]) ? $plugin_cookies->quality : $first;
    }

    /**
     * @param $quality
     * @param $plugin_cookies
     */
    public function set_quality_id($quality, $plugin_cookies)
    {
        $plugin_cookies->quality = $quality;
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

    /**
     * @return string
     */
    public function get_profile_name($plugin_cookies)
    {
        $profiles = $this->get_profiles($plugin_cookies);
        return $profiles[$this->get_profile_id($plugin_cookies)];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_profile_id($plugin_cookies)
    {
        $profiles = $this->get_profiles($plugin_cookies);
        reset($profiles);
        $first = key($profiles);
        return isset($plugin_cookies->profile, $quality[$plugin_cookies->profile]) ? $plugin_cookies->profile : $first;
    }

    /**
     * @param $profile
     * @param $plugin_cookies
     */
    public function set_profile_id($profile, $plugin_cookies)
    {
        $plugin_cookies->profile = $profile;
    }
}

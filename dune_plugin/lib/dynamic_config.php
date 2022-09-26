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
     * @param string $short_name
     * @return void
     */
    public function init_defaults($short_name)
    {
        $this->PluginShortName = $short_name;

        $this->features[ACCESS_TYPE] = -1;
        $this->features[SQUARE_ICONS] = false;
        $this->features[TV_FAVORITES_SUPPORTED] = true;
        $this->features[BALANCE_SUPPORTED] = false;
        $this->features[VOD_SUPPORTED] = false;
        $this->features[VOD_LAZY_LOAD] = false;
        $this->features[VOD_FAVORITES_SUPPORTED] = false;
        $this->features[VOD_QUALITY_SUPPORTED] = false;
        $this->features[VOD_FILTER_SUPPORTED] = false;
        $this->features[VOD_PARSE_PATTERN] = '';
        $this->features[DEVICE_OPTIONS] = array();
        $this->features[SERVER_OPTIONS] = false;
        $this->features[QUALITY_OPTIONS] = false;
        $this->features[USE_TOKEN_AS_ID] = false;

        // load defaults
        $default_streams = array();
        $default_streams[CU_TYPE] = 'shift';
        $default_streams[URL_TEMPLATE] = '';
        $default_streams[URL_ARC_TEMPLATE] = '{CU_SUBST}={START}&lutc={NOW}';
        $default_streams[CU_SUBST] = 'utc';
        $this->set_stream_params(HLS, $default_streams);

        $default_streams[CU_TYPE] = 'flussonic';
        $default_streams[CU_SUBST] = 'archive';
        $default_streams[URL_ARC_TEMPLATE] = '';
        $this->set_stream_params(MPEG, $default_streams);

        $default_parser = array();
        $default_parser[EPG_PARSER] = 'json';
        $default_parser[EPG_URL] = '';
        $default_parser[EPG_ROOT] = 'epg_data';
        $default_parser[EPG_START] = 'time';
        $default_parser[EPG_END] = 'time_to';
        $default_parser[EPG_NAME] = 'name';
        $default_parser[EPG_DESC] = 'descr';
        $default_parser[EPG_DATE_FORMAT] = '';
        $default_parser[EPG_USE_DURATION] = false;
        $default_parser[EPG_USE_MAPPER] = false;
        $default_parser[EPG_MAPPER_URL] = '';
        $default_parser[EPG_TIME_FORMAT] = '';
        $default_parser[EPG_TIMEZONE] = 0;
        $default_parser[EPG_ID_MAPPER] = array();

        $this->set_epg_params(EPG_FIRST, $default_parser);
        $this->set_epg_params(EPG_SECOND, $default_parser);

        $this->load_config();
    }
    /**
     * load configuration
     * @return void
     */
    public function load_config()
    {
        $path = $this->PluginShortName . '_config.json';
        $settings = HD::parse_json_file(get_install_path($path), true);
        hd_print("Load plugin settings: $path");

        // NLOHMANN_DEFINE_TYPE_INTRUSIVE(uri_stream, access_type, title, name, provider_url,
        //     playlist_template, uri_id_parse_pattern, uri_parse_pattern,
        //     square_icons, requested_token, use_token_as_id,
        //     streams_config, epg_params, servers_list, qualities_list, devices_list, profiles_list);


        $this->set_feature(ACCESS_TYPE, $settings[ACCESS_TYPE]);
        $this->set_feature(PLAYLIST_TEMPLATE, $settings[PLAYLIST_TEMPLATE]);
        $this->set_feature(URI_ID_PARSE_PATTERN, $settings[URI_ID_PARSE_PATTERN]);
        $this->set_feature(URI_PARSE_PATTERN, $settings[URI_PARSE_PATTERN]);
        $this->set_feature(SQUARE_ICONS, $settings[SQUARE_ICONS]);
        $this->set_feature(USE_TOKEN_AS_ID, $settings[USE_TOKEN_AS_ID]);

        foreach ($settings[STREAMS_CONFIG] as $config)
        {
            $param = $config[STREAM_TYPE] ? MPEG : HLS;
            foreach ($config as $key => $item) {
                //hd_print("$param: $key => $item");
                $this->set_stream_param($param, $key, $item);
            }
        }

        foreach ($settings[EPG_PARAMS] as $idx => $epg)
        {
            $param = $idx ? EPG_SECOND : EPG_FIRST;
            foreach ($epg as $key => $item) {
                //hd_print("$param: $key => $item");
                $this->set_epg_param($param, $key, $item);
            }
        }

        $servers = array();
        foreach ($settings[SERVERS_LIST] as $server)
        {
            $servers[] = array($server[LIST_ID] => $server[LIST_NAME]);
        }
        $this->set_servers($servers);

        $devices = array();
        foreach ($settings[DEVICES_LIST] as $device)
        {
            $devices[] = array($device[LIST_ID] => $device[LIST_NAME]);
        }
        $this->set_devices($devices);

        $qualities = array();
        foreach ($settings[QUALITIES_LIST] as $quality)
        {
            $qualities[] = array($quality[LIST_ID] => $quality[LIST_NAME]);
        }
        $this->set_qualities($qualities);

        $profiles = array();
        foreach ($settings[PROFILES_LIST] as $profile)
        {
            $profiles[] = array($profile[LIST_ID] => $profile[LIST_NAME]);
        }
        $this->set_profiles($profiles);
    }

    /**
     * @param string $type
     * @return mixed
     */
    public function get_feature($type)
    {
        return $this->features[$type];
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
     * @return mixed
     */
    public function get_stream_params($type)
    {
        return $this->stream_params[$type];
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
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server_id($plugin_cookies)
    {
        $servers = $this->get_servers($plugin_cookies);
        reset($servers);
        $first = key($servers);
        return isset($plugin_cookies->server, $quality[$plugin_cookies->server]) ? $plugin_cookies->server : $first;
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
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_device_id($plugin_cookies)
    {
        $devices = $this->get_devices($plugin_cookies);
        reset($devices);
        $first = key($devices);
        return isset($plugin_cookies->device, $quality[$plugin_cookies->device]) ? $plugin_cookies->device : $first;
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

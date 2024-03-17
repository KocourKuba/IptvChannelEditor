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
    private $domains = array();

    /**
     * load configuration
     * @return void
     */
    public function init_defaults()
    {
        hd_debug_print(null, true);
        $this->features[Plugin_Constants::VOD_QUALITY_SUPPORTED] = false; // currently supported only in edem
        $this->features[Plugin_Constants::VOD_FILTER_SUPPORTED] = false; // filter list screen
    }

    /**
     * load configuration
     * @return void
     */
    public function load_config()
    {
        hd_debug_print(null, true);

        $settings = HD::parse_json_file(get_install_path('config.json'), true);

        $this->set_feature(Plugin_Constants::ACCESS_TYPE, $settings[Plugin_Constants::ACCESS_TYPE]);
        $this->set_feature(Plugin_Constants::PROVIDER_API_URL, $settings[Plugin_Constants::PROVIDER_API_URL]);
        $this->set_feature(Plugin_Constants::BALANCE_SUPPORTED, $settings[Plugin_Constants::BALANCE_SUPPORTED]);
        $this->set_feature(Plugin_Constants::PLAYLIST_TEMPLATES, $settings[Plugin_Constants::PLAYLIST_TEMPLATES]);
        $this->set_feature(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX, $settings[Plugin_Constants::PLAYLIST_TEMPLATE_INDEX]);
        $this->set_feature(Plugin_Constants::VOD_ENGINE, $settings[Plugin_Constants::VOD_ENGINE]);
        $this->set_feature(Plugin_Constants::VOD_TEMPLATES, $settings[Plugin_Constants::VOD_TEMPLATES]);

        HD::set_dune_user_agent($settings[Plugin_Constants::USER_AGENT]);
        HD::set_plugin_dev_code($settings[Plugin_Constants::DEV_CODE]);

        foreach ($settings[Plugin_Constants::STREAMS_CONFIG] as $config) {
            $param_idx = $config[Stream_Params::STREAM_TYPE];
            $this->set_stream_params($param_idx, $config);
        }

        foreach ($settings[Plugin_Constants::EPG_PARAMS] as $epg) {
            $param_idx = $epg[Epg_Params::EPG_PARAM];
            $this->set_epg_params($param_idx, $epg);
        }

        $this->set_feature(Plugin_Constants::EPG_CUSTOM_SOURCE, $settings[Plugin_Constants::EPG_CUSTOM_SOURCE]);

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

        $domains = array();
        foreach ($settings[Plugin_Constants::DOMAINS_LIST] as $pair) {
            $domains[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        $this->set_domains($domains);

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
        return isset($this->stream_params[$type][$param]) ? $this->stream_params[$type][$param] : "";
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
     * @return array|null
     */
    public function get_epg_params($type)
    {
        return isset($this->epg_parser_params[$type]) ? $this->epg_parser_params[$type] : null;
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
    public function get_servers()
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
    public function get_devices()
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
    public function get_qualities()
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
    public function get_profiles()
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
     * @return array
     */
    public function get_domains()
    {
        return $this->domains;
    }

    /**
     * @param array $val
     */
    public function set_domains($val)
    {
        $this->domains = $val;
    }
}

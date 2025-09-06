<?php

class dynamic_config
{
    // info
    public $plugin_info;

    // features constants
    protected $parameters = array();
    protected $features = array();
    protected $stream_parameters = array();
    protected $epg_parser_parameters = array();
    protected $vod_templates = array();
    protected $servers = array();
    protected $devices = array();
    protected $qualities = array();
    protected $profiles = array();
    protected $domains = array();

    /**
     * load configuration
     * @return void
     */
    public function init_defaults()
    {
        hd_debug_print(null, true);
    }

    /**
     * load configuration
     * @return void
     */
    public function load_config_parameters()
    {
        hd_debug_print(null, true);

        $this->parameters = parse_json_file(get_install_path('config.json'));

        $this->features[Plugin_Constants::ACCESS_TYPE] = $this->get_parameter(Plugin_Constants::ACCESS_TYPE);
        $this->features[Plugin_Constants::PROVIDER_API_URL] = $this->get_parameter(Plugin_Constants::PROVIDER_API_URL);
        $this->features[Plugin_Constants::BALANCE_SUPPORTED] = $this->get_parameter(Plugin_Constants::BALANCE_SUPPORTED, false);
        $this->features[Plugin_Constants::PLAYLIST_TEMPLATES] = $this->get_parameter(Plugin_Constants::PLAYLIST_TEMPLATES, array());
        $this->features[Plugin_Constants::PLAYLIST_TEMPLATE_INDEX] = $this->get_parameter(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX);
        $this->features[Plugin_Constants::VOD_ENGINE] = $this->get_parameter(Plugin_Constants::VOD_ENGINE);
        $this->features[Plugin_Constants::VOD_TEMPLATES] = $this->get_parameter(Plugin_Constants::VOD_TEMPLATES, array());
        $this->features[Plugin_Constants::VOD_FILTER_SUPPORTED] = $this->get_parameter(Plugin_Constants::VOD_FILTER_SUPPORTED, false);
        $this->features[Plugin_Constants::VOD_QUALITY_SUPPORTED] = $this->get_parameter(Plugin_Constants::VOD_QUALITY_SUPPORTED, false);
        $this->features[Plugin_Constants::VOD_AUDIO_SUPPORTED] = $this->get_parameter(Plugin_Constants::VOD_AUDIO_SUPPORTED, false);
        $this->features[Plugin_Constants::VOD_FILTERS] = $this->get_parameter(Plugin_Constants::VOD_FILTERS, array());

        HD::set_dune_user_agent($this->get_parameter(Plugin_Constants::USER_AGENT));
        HD::set_plugin_dev_code($this->get_parameter(Plugin_Constants::DEV_CODE));

        foreach ($this->get_parameter(Plugin_Constants::STREAMS_CONFIG, array()) as $config) {
            $param_idx = $config[Stream_Params::STREAM_TYPE];
            $this->stream_parameters[$param_idx] = $config;
        }

        foreach ($this->get_parameter(Plugin_Constants::EPG_PARAMS, array()) as $epg) {
            $param_idx = $epg[Epg_Params::EPG_PARAM];
            $this->epg_parser_parameters[$param_idx] = $epg;
        }

        $this->features[Plugin_Constants::EPG_CUSTOM_SOURCE] = $this->get_parameter(Plugin_Constants::EPG_CUSTOM_SOURCE, array());

        $servers = array();
        foreach ($this->get_parameter(Plugin_Constants::SERVERS_LIST, array()) as $pair) {
            $servers[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($servers)) {
            $this->set_servers($servers);
        }

        $devices = array();
        foreach ($this->get_parameter(Plugin_Constants::DEVICES_LIST, array()) as $pair) {
            $devices[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($devices)) {
            $this->set_devices($devices);
        }

        $qualities = array();
        foreach ($this->get_parameter(Plugin_Constants::QUALITIES_LIST, array()) as $pair) {
            $qualities[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($qualities)) {
            $this->set_qualities($qualities);
        }

        $profiles = array();
        foreach ($this->get_parameter(Plugin_Constants::PROFILES_LIST, array()) as $pair) {
            $profiles[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($profiles)) {
            $this->set_profiles($profiles);
        }

        $domains = array();
        foreach ($this->get_parameter(Plugin_Constants::DOMAINS_LIST, array()) as $pair) {
            $domains[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($domains)) {
            $this->set_domains($domains);
        }
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
     * @param string $param
     * @return mixed
     */
    public function get_stream_parameter($type, $param)
    {
        return isset($this->stream_parameters[$type][$param]) ? $this->stream_parameters[$type][$param] : "";
    }

    /**
     * @param string $type
     * @return array|null
     */
    public function get_epg_parameters($type)
    {
        return isset($this->epg_parser_parameters[$type]) ? $this->epg_parser_parameters[$type] : null;
    }

    /**
     * @param string $type
     * @param string $param
     * @return mixed
     */
    public function get_epg_parameter($type, $param)
    {
        return isset($this->epg_parser_parameters[$type][$param]) ? $this->epg_parser_parameters[$type][$param] : null;
    }

    /**
     * @param string $type
     * @param string $param
     * @param mixed $val
     */
    public function set_epg_parameter($type, $param, $val)
    {
        $this->epg_parser_parameters[$type][$param] = $val;
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

    /**
     * @param $key string
     * @param $default mixed
     * @return mixed|string
     */
    protected function get_parameter($key, $default = '')
    {
        if (isset($this->parameters[$key])) {
            return $this->parameters[$key];
        }

        return $default;
    }
}

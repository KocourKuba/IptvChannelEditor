<?php

class dynamic_config
{
    // info
    public $plugin_info;

    // features constants
    protected $features = array();
    protected $stream_params = array();
    protected $epg_parser_params = array();
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
    public function load_config()
    {
        hd_debug_print(null, true);

        $settings = parse_json_file(get_install_path('config.json'));

        $this->set_feature_from_settings(Plugin_Constants::ACCESS_TYPE, $settings);
        $this->set_feature_from_settings(Plugin_Constants::PROVIDER_API_URL, $settings);
        $this->set_feature_from_settings(Plugin_Constants::BALANCE_SUPPORTED, $settings, false);
        $this->set_feature_from_settings(Plugin_Constants::PLAYLIST_TEMPLATES, $settings, array());
        $this->set_feature_from_settings(Plugin_Constants::PLAYLIST_TEMPLATE_INDEX, $settings);
        $this->set_feature_from_settings(Plugin_Constants::VOD_ENGINE, $settings);
        $this->set_feature_from_settings(Plugin_Constants::VOD_TEMPLATES, $settings, array());
        $this->set_feature_from_settings(Plugin_Constants::VOD_FILTER_SUPPORTED, $settings, false);
        $this->set_feature_from_settings(Plugin_Constants::VOD_QUALITY_SUPPORTED, $settings, false);
        $this->set_feature_from_settings(Plugin_Constants::VOD_AUDIO_SUPPORTED, $settings, false);
        $this->set_feature_from_settings(Plugin_Constants::VOD_FILTERS, $settings, array());

        HD::set_dune_user_agent($this->read_settings(Plugin_Constants::USER_AGENT, $settings));
        HD::set_plugin_dev_code($this->read_settings(Plugin_Constants::DEV_CODE, $settings));

        foreach ($this->read_settings(Plugin_Constants::STREAMS_CONFIG, $settings, array()) as $config) {
            $param_idx = $config[Stream_Params::STREAM_TYPE];
            $this->set_stream_params($param_idx, $config);
        }

        foreach ($this->read_settings(Plugin_Constants::EPG_PARAMS, $settings, array()) as $epg) {
            $param_idx = $epg[Epg_Params::EPG_PARAM];
            $this->set_epg_params($param_idx, $epg);
        }

        $this->set_feature_from_settings(Plugin_Constants::EPG_CUSTOM_SOURCE, $settings, array());

        $servers = array();
        foreach ($this->read_settings(Plugin_Constants::SERVERS_LIST, $settings, array()) as $pair) {
            $servers[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($servers)) {
            $this->set_servers($servers);
        }

        $devices = array();
        foreach ($this->read_settings(Plugin_Constants::DEVICES_LIST, $settings, array()) as $pair) {
            $devices[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($devices)) {
            $this->set_devices($devices);
        }

        $qualities = array();
        foreach ($this->read_settings(Plugin_Constants::QUALITIES_LIST, $settings, array()) as $pair) {
            $qualities[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($qualities)) {
            $this->set_qualities($qualities);
        }

        $profiles = array();
        foreach ($this->read_settings(Plugin_Constants::PROFILES_LIST, $settings, array()) as $pair) {
            $profiles[$pair[Plugin_Constants::LIST_ID]] = $pair[Plugin_Constants::LIST_NAME];
        }
        if (!empty($profiles)) {
            $this->set_profiles($profiles);
        }

        $domains = array();
        foreach ($this->read_settings(Plugin_Constants::DOMAINS_LIST, $settings, array()) as $pair) {
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
     * @return mixed
     */
    public function get_epg_param($type, $param)
    {
        return isset($this->epg_parser_params[$type][$param]) ? $this->epg_parser_params[$type][$param] : null;
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
     * @param string $key
     * @param array $settings
     * @param mixed $default
     */
    protected function set_feature_from_settings($key, $settings, $default = '')
    {
        $this->features[$key] = $this->read_settings($key, $settings, $default);
    }

    /**
     * @param $key string
     * @param $settings array
     * @param $default mixed
     * @return mixed|string
     */
    protected function read_settings($key, $settings, $default = '')
    {
        if (isset($settings[$key])) {
            return $settings[$key];
        }

        return $default;
    }
}

<?php
require_once 'default_config.php';

class KineskopPluginConfig extends Default_Config
{
    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://knkp.in/{LOGIN}/{PASSWORD}/{SERVER}/1');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>[^/]+)/(?<host>[^/]+)/(?<id>[^/]+)/(?<token>[^/]+)\.m3u8$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.esalecrm.net/kineskop/epg/{ID}.json');
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array(
            'DE',
            'PL',
            'US',
            'RU'
        );
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server_id($plugin_cookies)
    {
        return isset($plugin_cookies->server) ? $plugin_cookies->server : 0;
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
     * @param $plugin_cookies
     * @return string|null
     */
    public function get_server($plugin_cookies)
    {
        $server = $this->get_server_id($plugin_cookies);
        $server_ops = $this->get_server_opts($plugin_cookies);
        return strtolower($server_ops[$server]);
    }
}

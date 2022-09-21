<?php
require_once 'default_config.php';

class TvteamPluginConfig extends Default_Config
{
    protected $server_opts;

    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://tv.team/pl/11/{PASSWORD}/playlist.m3u8');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<id>.+)/mono\.m3u8\?token=(?<token>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'index');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://tv.team/{ID}.json');
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return $this->server_opts[0];
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
     * @return string
     */
    protected function get_subst_server($plugin_cookies)
    {
        return $this->server_opts[1][$this->get_server_id($plugin_cookies)];
    }
}

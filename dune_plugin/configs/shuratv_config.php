<?php
require_once 'default_config.php';

class ShuratvPluginConfig extends Default_Config
{
    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.tvshka.net/?uid={PASSWORD}&srv={SERVER_ID}&type=halva');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|');

        $this->set_stream_param(HLS,CU_SUBST, 'archive');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.propg.net/{ID}/epg2/{DATE}');
        $this->set_epg_param(EPG_FIRST,EPG_DATE_FORMAT, '{YEAR}-{MONTH}-{DAY}');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'start');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'stop');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'epg');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'desc');
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $now_ts = time();
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "archive=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array('1', '2');
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
}

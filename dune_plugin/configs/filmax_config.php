<?php
require_once 'default_config.php';

class FilmaxPluginConfig extends Default_Config
{
    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://lk.filmax-tv.ru/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8');
        $this->set_feature(PLAYLIST_TEMPLATE2, 'http://epg.esalecrm.net/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<token>.+)/index\.m3u8\?token=(?<password>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'archive');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/index.m3u8?token={PASSWORD}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={PASSWORD}');

        $this->set_stream_param(MPEG,CU_TYPE, 'flussonic');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.ts?token={PASSWORD}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.esalecrm.net/filmax/epg/{ID}.json');
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if (!preg_match('|^#EXTINF:.+tvg-name="(?<id>[^"]+)"|', $iValue, $m_id)) continue;
            //hd_print("tvg-name: " . $m_id[M_CH_ID]);

            if (preg_match($this->get_feature(URI_PARSE_TEMPLATE), $m3u_lines[$i + 1], $matches)
                || preg_match($this->get_feature(URI_PARSE_TEMPLATE), $m3u_lines[$i + 2], $matches)) {
                $pl_entries[$m_id[M_ID]] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
        }

        return $pl_entries;
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array(
            1 => 'Германия',
            2 => 'Польша',
            3 => 'Москва',
            4 => 'Франция',
            5 => 'Санкт-Петербург',
            6 => 'Екатеринбург',
            7 => 'Казахстан',
            8 => 'Москва 2',
            9 => 'Москва 3',
            10 => 'Санкт-Петербург 2',
            11 => 'Чехия',
            12 => 'Тула'
        );
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server_id($plugin_cookies)
    {
        return isset($plugin_cookies->server) && $plugin_cookies->server !== 0 ? $plugin_cookies->server : 1;
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

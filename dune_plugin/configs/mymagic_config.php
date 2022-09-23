<?php
require_once 'lib/default_config.php';

class mymagic_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(SERVER_OPTIONS, true);
        $this->set_feature(QUALITY_OPTIONS, true);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.mymagic.tv/srv/{SERVER_ID}/{QUALITY_ID}/{LOGIN}/{PASSWORD}/tv.m3u');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>[^/]+)/(?<token>.+)$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/magic/epg/{ID}.json');

        $servers = array(
            'По умолчанию',
            'Германия 1',
            'Чехия',
            'Германия 2',
            'Испания',
            'Нидерланды',
            'Франция',
        );

        $this->set_servers($servers);
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
        $skip_next = false;
        foreach ($m3u_lines as $i => $iValue) {
            if ($skip_next) {
                $skip_next = false;
                continue;
            }

            if (preg_match('|^#EXTINF:.+CUID="(?<id>\d+)"|', $iValue, $m_id)
                && preg_match($this->get_feature(URI_PARSE_TEMPLATE), $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id[M_ID]] = $matches;
                $skip_next = true;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
        }

        return $pl_entries;
    }
}

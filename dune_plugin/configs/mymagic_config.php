<?php
require_once 'lib/default_config.php';

class mymagic_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(ACCESS_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.mymagic.tv/srv/{SERVER_ID}/{QUALITY_ID}/{LOGIN}/{PASSWORD}/tv.m3u');
        $this->set_feature(URI_ID_PARSE_PATTERN, '^#EXTINF:.+CUID="(?<id>\d+)"');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>[^/]+)/(?<token>.+)$');

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
}

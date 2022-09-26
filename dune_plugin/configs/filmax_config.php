<?php
require_once 'lib/default_config.php';

class filmax_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(SERVER_OPTIONS, true);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://lk.filmax-tv.ru/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8');
        $this->set_feature(PLAYLIST_TEMPLATE2, 'http://epg.esalecrm.net/{LOGIN}/{PASSWORD}/hls/p{SERVER_ID}/playlist.m3u8');
        $this->set_feature(URI_ID_PARSE_PATTERN, '^#EXTINF:.+tvg-name="(?<id>[^"]+)"');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/(?<token>.+)/index\.m3u8\?token=(?<password>.+)$');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'archive');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/index.m3u8?token={PASSWORD}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={PASSWORD}');

        $this->set_stream_param(MPEG,CU_TYPE, 'flussonic');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.ts?token={PASSWORD}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.esalecrm.net/filmax/epg/{ID}.json');
        $servers = array(
            'Германия',
            'Польша',
            'Москва',
            'Франция',
            'Санкт-Петербург',
            'Екатеринбург',
            'Казахстан',
            'Москва 2',
            'Москва 3',
            'Санкт-Петербург 2',
            'Чехия',
            'Тула'
        );
        $this->set_servers($servers);
    }
}

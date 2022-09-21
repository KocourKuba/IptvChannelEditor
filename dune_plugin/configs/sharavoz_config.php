<?php
require_once 'default_config.php';

class SharavozPluginConfig extends Default_Config
{
    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://www.spr24.net/iptv/p/{PASSWORD}/Sharavoz.Tv.navigator-ott.m3u');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<id>.+)/(?:.*)\?token=(?<token>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL, 'http://api.program.spr24.net/api/program?epg={ID}');
        $this->set_epg_param(EPG_SECOND,EPG_URL, 'http://epg.arlekino.tv/api/program?epg={ID}');
    }
}

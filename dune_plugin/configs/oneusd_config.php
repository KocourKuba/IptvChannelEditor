﻿<?php
require_once 'lib/default_config.php';

class oneusd_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://1usd.tv/pl-{PASSWORD}-hls');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<id>.+)/mono\.m3u8\?token=(?<token>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'index');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://tv.team/{ID}.json');
    }
}

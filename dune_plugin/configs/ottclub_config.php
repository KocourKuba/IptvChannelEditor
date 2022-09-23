﻿<?php
require_once 'lib/default_config.php';

class ottclub_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://myott.top/playlist/{PASSWORD}/m3u');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/stream/(?<token>.+)/(?<id>.+)\.m3u8$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/stream/{TOKEN}/{ID}.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://myott.top/api/channel/{ID}');
    }
}

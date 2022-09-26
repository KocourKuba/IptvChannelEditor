<?php
require_once 'lib/default_config.php';

class sharatv_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://tvfor.pro/g/{LOGIN}:{PASSWORD}/1/playlist.m3u');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/(?<id>.+)/(?<token>.+)$');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/{TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/shara-tv/epg/{ID}.json');
    }
}

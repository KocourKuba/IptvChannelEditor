<?php
require_once 'lib/default_config.php';

class russkoetv_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://russkoetv.tv/play/{PASSWORD}.m3u8');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/s/(?<token>.+)/(?<id>.+)\.m3u8$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_URL, 'http://protected-api.com/epg/{ID}/?date=');
    }
}

<?php
require_once 'lib/default_config.php';

class kineskop_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(SERVER_OPTIONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://knkp.in/{LOGIN}/{PASSWORD}/{SERVER}/1');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>[^/]+)/(?<host>[^/]+)/(?<id>[^/]+)/(?<token>[^/]+)\.m3u8$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.esalecrm.net/kineskop/epg/{ID}.json');
        $this->set_servers(array('de', 'pl', 'us', 'ru'));
    }
}

<?php
require_once 'lib/default_config.php';

class shuratv_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.tvshka.net/?uid={PASSWORD}&srv={SERVER_ID}&type=halva');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$');

        $this->set_stream_param(HLS,CU_SUBST, 'archive');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.propg.net/{ID}/epg2/{DATE}');
        $this->set_epg_param(EPG_FIRST,EPG_DATE_FORMAT, '{YEAR}-{MONTH}-{DAY}');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'start');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'stop');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'epg');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'desc');

        $this->set_servers(array('1', '2'));
    }
}

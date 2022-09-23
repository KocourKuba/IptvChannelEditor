<?php
require_once 'lib/default_config.php';

class onecent_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://only4.tv/pl/{PASSWORD}/102/only4tv.m3u8');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<id>.+)/index\.m3u8\?token=(?<token>.+)$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL, 'http://epg.iptvx.one/api/id/{ID}.json');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, 'ch_programme');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'start');
        $this->set_epg_param(EPG_FIRST,EPG_END, '');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'title');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'description');
        $this->set_epg_param(EPG_FIRST,EPG_TIME_FORMAT, '{DAY}-{MONTH}-{YEAR} {HOUR}:{MIN}'); // 'd-m-Y H:i'
        $this->set_epg_param(EPG_FIRST,EPG_TIMEZONE, 3); // // iptvx.one uses moscow time (UTC+3)

        $this->set_epg_param(EPG_SECOND,EPG_URL,'http://epg.drm-play.ml/iptvx.one/epg/{ID}.json');
    }
}

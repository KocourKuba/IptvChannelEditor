<?php
require_once 'lib/default_config.php';

class iptvonline_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://iptv.online/play/{PASSWORD}/m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>[^/]+)/play/(?<id>[^/]+)/(?<token>[^/]+)/video\.m3u8$');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'video');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/play/{ID}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/play/{ID}/{TOKEN}/mpegts');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/play/{ID}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.ts');

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

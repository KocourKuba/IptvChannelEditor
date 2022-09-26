<?php
require_once 'lib/default_config.php';

class lightiptv_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://lightiptv.cc/playlist/hls/{PASSWORD}.m3u');
        $this->set_feature(URI_ID_PARSE_PATTERN, '^#EXTINF:.+tvg-id="(?<id>[^"]+)"');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>[^/]+)/(?<token>[^/]+)/video\.m3u8\?token=(?<password>.+)$');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,CU_SUBST, 'video');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={PASSWORD}');

        $this->set_stream_param(MPEG,CU_SUBST, 'timeshift_abs');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{TOKEN}/{CU_SUBST}-{START}.m3u8?token={PASSWORD}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/lightiptv/epg/{ID}.json');
        $this->set_epg_param(EPG_SECOND,EPG_URL,'http://epg.ott-play.com/lightiptv/epg/{ID}.json');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $channel->set_ext_param(M_PASSWORD, $this->get_password($plugin_cookies));
        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }
}

<?php
require_once 'lib/default_config.php';

class viplime_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://cdntv.online/high/{PASSWORD}/playlist.m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.m3u8');

        $this->set_stream_param(MPEG,CU_TYPE, 'shift');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{QUALITY_ID}/{TOKEN}/{ID}.mpeg');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/viplime/epg/{ID}.json');

        $this->set_qualities(array('Высокое', 'Среднее', 'Низкое', 'Адаптивное', 'Оптимальное'));
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
        $channel->set_ext_param(M_QUALITY, $this->get_qualities($plugin_cookies));

        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }
}

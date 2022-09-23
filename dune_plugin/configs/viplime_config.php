<?php
require_once 'lib/default_config.php';

class viplime_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_PIN);
        $this->set_feature(QUALITY_OPTIONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://cdntv.online/high/{PASSWORD}/playlist.m3u8');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8');

        $this->set_stream_param(MPEG,CU_TYPE, 'shift');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.mpeg');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/viplime/epg/{ID}.json');
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
        $ext_params = $channel->get_ext_params();
        $ext_params[M_QUALITY] = $this->get_quality($plugin_cookies);
        $channel->set_ext_params($ext_params);

        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_quality_opts($plugin_cookies)
    {
        return array('Высокое', 'Среднее', 'Низкое', 'Адаптивное', 'Оптимальное');
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_quality_id($plugin_cookies)
    {
        return isset($plugin_cookies->quality) ? $plugin_cookies->quality : 0;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_quality($plugin_cookies)
    {
        $quality = array('high', 'middle', 'low', 'variant', 'hls');
        return $quality[$this->get_quality_id($plugin_cookies)];
    }

    /**
     * @param $quality
     * @param $plugin_cookies
     */
    public function set_quality_id($quality, $plugin_cookies)
    {
        $plugin_cookies->quality = $quality;
    }
}

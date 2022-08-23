<?php
require_once 'default_config.php';

class ViplimePluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://cdntv.online/high/%s/playlist.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(QUALITY_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.mpeg');

        $this->set_epg_param('first','epg_url','http://epg.drm-play.ml/viplime/epg/{CHANNEL}.json');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $url = $channel->get_streaming_url();
        if (empty($url)) {
            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $template = $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $template = $this->get_feature(MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown play format: " . $this->get_format($plugin_cookies));
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}', '{QUALITY}'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token'], $this->get_quality_value($plugin_cookies)),
                $template);
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = $this->get_password($plugin_cookies);
        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
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
    public function get_quality($plugin_cookies)
    {
        return isset($plugin_cookies->quality) ? $plugin_cookies->quality : 0;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_quality_value($plugin_cookies)
    {
        $quality = array('high', 'middle', 'low', 'variant', 'hls');
        return $quality[$this->get_quality($plugin_cookies)];
    }

    /**
     * @param $quality
     * @param $plugin_cookies
     */
    public function set_quality($quality, $plugin_cookies)
    {
        $plugin_cookies->quality = $quality;
    }
}

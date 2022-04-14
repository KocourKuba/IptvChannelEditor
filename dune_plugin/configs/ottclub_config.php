<?php
require_once 'default_config.php';

class OttclubPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://myott.top/playlist/%s/m3u';
    const API_HOST = 'http://my-ott.top/api';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/stream/(?<token>.+)/(?<id>.+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/stream/{TOKEN}/{ID}.m3u8');

        $this->set_epg_param('epg_url', self::API_HOST . '/channel/{CHANNEL}');
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
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);
        // hd_print("Stream url:  " . $url);
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

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }
}

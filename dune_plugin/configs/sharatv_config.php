<?php
require_once 'default_config.php';

class SharatvPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://tvfor.pro/g/%s:%s/1/playlist.m3u';
    const API_HOST = 'http://technic.cf/epg-shara-tv';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/{TOKEN}');

        $this->set_epg_param('epg_url', self::API_HOST . '/epg_day?id={CHANNEL}&day={DATE}', 'first');
        $this->set_epg_param('epg_root', 'data', 'first');
        $this->set_epg_param('start', 'begin', 'first');
        $this->set_epg_param('end', 'end', 'first');
        $this->set_epg_param('title', 'title', 'first');
        $this->set_epg_param('description', 'description', 'first');
        $this->set_epg_param('date_format', 'Y.m.d', 'first');
        $this->set_epg_param('use_epg_mapper', true, 'first');
        $this->set_epg_param('epg_mapper_url', self::API_HOST . '/channels', 'first');
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

        // shara tv does not support hls, only mpeg-ts
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

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $login, $password);
    }
}

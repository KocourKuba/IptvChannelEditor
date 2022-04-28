<?php
require_once 'default_config.php';

class SharatvPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://tvfor.pro/g/%s:%s/1/playlist.m3u';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/{TOKEN}');

        $this->set_epg_param('first','epg_url','http://technic.cf/epg-shara-tv/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('first','epg_root', 'data');
        $this->set_epg_param('first','epg_start', 'begin');
        $this->set_epg_param('first','epg_end', 'end');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_date_format', 'Y.m.d');
        $this->set_epg_param('first','epg_use_mapper', true);
        $this->set_epg_param('first','epg_mapper_url', 'http://technic.cf/epg-shara-tv/channels');
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

        $login = isset($this->embedded_account->login) ? $this->embedded_account->login : $plugin_cookies->login;
        $password = isset($plugin_cookies->password) ? $this->embedded_account->password : $plugin_cookies->password;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $login, $password);
    }
}

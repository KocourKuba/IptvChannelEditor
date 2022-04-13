<?php
require_once 'default_config.php';

class ShuratvPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.tvshka.net/?uid=%s&srv=%d&type=halva';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');
        $this->set_feature(SERVER_SUPPORTED, true);

        $this->set_epg_param('epg_root', '');
        $this->set_epg_param('start', 'start_time');
        $this->set_epg_param('end', 'duration');
        $this->set_epg_param('title', 'name');
        $this->set_epg_param('description', 'text');
        $this->set_epg_param('use_duration', true);
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

        if ($this->get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('/hls/pl.m3u8', '', $url);
        }

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
            hd_print("User password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password, $this->get_server($plugin_cookies));
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $now_ts = time();
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "archive=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array('1', '2');
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server($plugin_cookies)
    {
        return isset($plugin_cookies->server) ? $plugin_cookies->server : 0;
    }

    /**
     * @param string $type
     * @param string $id
     * @param int $day_start_ts
     * @param $plugin_cookies
     * @return string|null
     */
    public function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            return sprintf('http://s1.tvshka.net/%s/epg/range14-7.json', $id);
        }

        return null;
    }
}

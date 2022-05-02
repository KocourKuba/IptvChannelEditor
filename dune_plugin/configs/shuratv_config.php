<?php
require_once 'default_config.php';

class ShuratvPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.tvshka.net/?uid=%s&srv=%d&type=halva';
    const API_HOST = 'http://s1.tvshka.net';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/~{TOKEN}/{ID}');
        $this->set_feature(SERVER_SUPPORTED, true);

        $this->set_epg_param('first','epg_url','http://s1.tvshka.net/{CHANNEL}/epg/range14-7.json');
        $this->set_epg_param('first','epg_root', '');
        $this->set_epg_param('first','epg_start', 'start_time');
        $this->set_epg_param('first','epg_end', 'duration');
        $this->set_epg_param('first','epg_title', 'name');
        $this->set_epg_param('first','epg_desc', 'text');
        $this->set_epg_param('first','epg_use_duration', true);
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
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token']),
                $template);
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
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
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;
        if (empty($password)) {
            hd_print("User password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password, $this->get_server($plugin_cookies));
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
}

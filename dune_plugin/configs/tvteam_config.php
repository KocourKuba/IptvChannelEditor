<?php
require_once 'default_config.php';

class TvteamPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://tv.team/pl/11/%s/playlist.m3u8';

    protected $server_opts;

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/mono\.m3u8\?token=(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/mono.m3u8?token={TOKEN}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/{ID}/index-{START}-7200.m3u8?token={TOKEN}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{ID}/archive-{START}-7200.m3u8?token={TOKEN}');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('first','epg_url','http://tv.team/{CHANNEL}.json');

        $this->server_opts = array(
            array(
                'Все (кроме RU, BY)',
                'DE, RU',
                'DE, RU, BY, MD',
                'DE, UA, BY, MD',
                'FR, DE, RU, BY',
                'HL',
                'RU, BY',
                'USA 1',
                'USA 2'
            ),
            array(
                '3.troya.tv',
                '4.troya.tv',
                '9.troya.tv',
                '8.troya.tv',
                '7.troya.tv',
                '02.tv.team',
                '10.troya.tv',
                '01.tv.team',
                '2.troya.tv'
            )
        );
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
        if (!empty($url)) {
            $url = ((int)$archive_ts <= 0) ?: static::UpdateArchiveUrlParams($url, $archive_ts);
        } else {
            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $template = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_HLS : MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $template = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $host = explode($ext_params['subdomain'], ':');
            if (isset($host[1])) {
                $domain = $this->get_subst_server($plugin_cookies) . ":" . $host[1];
            } else {
                $domain = $ext_params['subdomain'];
            }
            $url = str_replace(array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}'),
                array($domain, $channel->get_channel_id(), $ext_params['token'], $archive_ts),
                $template);
        }

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
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return $this->server_opts[0];
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
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server($server, $plugin_cookies)
    {
        $plugin_cookies->server = $server;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_subst_server($plugin_cookies)
    {
        return $this->server_opts[1][$this->get_server($plugin_cookies)];
    }
}

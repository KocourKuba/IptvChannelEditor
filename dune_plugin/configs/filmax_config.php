<?php
require_once 'default_config.php';

class FilmaxPluginConfig extends Default_Config
{
    const PLAYLIST_TV1_URL = 'http://lk.filmax-tv.ru/%s/%s/hls/p%s/playlist.m3u8';
    const PLAYLIST_TV2_URL = 'http://epg.esalecrm.net/%s/%s/hls/p%s/playlist.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<token>.+)/index\.m3u8\?token=(?<password>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{TOKEN}/index.m3u8?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/{TOKEN}/archive-{START}-10800.m3u8?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{TOKEN}/archive-{START}-10800.ts?token={PASSWORD}');

        $this->set_epg_param('first','epg_url','http://epg.esalecrm.net/filmax/epg/{CHANNEL}.json');
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
                    $template = ((int)$archive_ts > 0) ? $this->get_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS) : $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $template = ((int)$archive_ts > 0) ? $this->get_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG) : $this->get_feature(MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(array('{DOMAIN}', '{TOKEN}', '{PASSWORD}', '{START}'),
                array($ext_params['subdomain'], $ext_params['token'], $ext_params['password'], $archive_ts),
                $template);
        }

        hd_print("Stream url:  $url");

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

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);
        $server = $this->get_server($plugin_cookies) + 1;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV1_URL, $login, $password, $server);
            case 'tv2':
                return sprintf(self::PLAYLIST_TV2_URL, $login, $password, $server);
        }

        return '';
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information for: $this->PLUGIN_SHOW_NAME");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if (!preg_match('|^#EXTINF:.+tvg-name="(?<id>[^"]+)"|', $iValue, $m_id)) continue;
            //hd_print("tvg-name: " . $m_id['id']);

            if (preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $m3u_lines[$i + 1], $matches)
                || preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $m3u_lines[$i + 2], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
        }

        return $pl_entries;
    }

    /**
     * @param string $channel_id
     * @param array $ext_params
     * @return array|mixed|string|string[]
     */
    public function UpdateStreamUrlID($channel_id, $ext_params)
    {
        // token used as channel id
        return str_replace('{TOKEN}', $ext_params['token'], $this->get_feature(MEDIA_URL_TEMPLATE_HLS));
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array(
            'Германия',
            'Польша',
            'Москва',
            'Франция',
            'Санкт-Петербург',
            'Екатеринбург',
            'Казахстан',
            'Москва 2',
            'Москва 3',
            'Санкт-Петербург 2',
            'Чехия',
            'Тула'
        );
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
}

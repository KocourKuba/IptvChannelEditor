<?php
require_once 'default_config.php';

class MymagicPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.mymagic.tv/srv/%s/%s/%s/%s/tv.m3u';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>[^/]+)/(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{TOKEN}');
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(SECONDARY_EPG, true);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(QUALITY_SUPPORTED, true);

        $this->set_epg_param('first','epg_url','http://epg.drm-play.ml/magic/epg/{CHANNEL}.json');
        $this->set_epg_param('second','epg_url','http://epg.esalecrm.net/magic/epg/{CHANNEL}.json');
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
            // http://ost.mymagic.tv/vLm0zdTg_XR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL3ZpZGV/video.m3u8
            // http://ost.mymagic.tv/vLm0zdTg_XR2LmZ1bjo5OTg2L1BlcnZpeWthbmFsL3ZpZGV
            // hls archive url completely different, make it from scratch
            $template = $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
            $ext_params = $channel->get_ext_params();

            $url = str_replace(array('{DOMAIN}', '{TOKEN}'), array($ext_params['subdomain'], $ext_params['token']), $template);
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

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);
        $server = $this->get_server($plugin_cookies);
        $quality = $this->get_quality($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $server, $quality, $login, $password);
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
        $skip_next = false;
        foreach ($m3u_lines as $i => $iValue) {
            if ($skip_next) {
                $skip_next = false;
                continue;
            }

            if (preg_match('|^#EXTINF:.+CUID="(?<id>\d+)"|', $iValue, $m_id)
                && preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
                $skip_next = true;
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
            'По умолчанию',
            'Германия 1',
            'Чехия',
            'Германия 2',
            'Испания',
            'Нидерланды',
            'Франция',
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

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_quality_opts($plugin_cookies)
    {
        return array('Среднее', 'Высокое');
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
        $quality = array('0', '1');
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

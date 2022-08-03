<?php
require_once 'default_config.php';

class LightiptvPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://lightiptv.cc/playlist/hls/%s.m3u';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/video\.m3u8\?token=(?<password>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/{TOKEN}/video-{START}-10800.m3u8?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{TOKEN}/mpegts?token={PASSWORD}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{TOKEN}/timeshift_abs-{START}.m3u8?token={PASSWORD}');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('first','epg_url','http://epg.esalecrm.net/lightiptv/epg/{CHANNEL}.json');
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
            $url = static::UpdateArchiveUrlParams($url, $archive_ts);
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

            $password = $this->get_password($plugin_cookies);
            $ext_params = $channel->get_ext_params();
            $url = str_replace(array('{DOMAIN}', '{TOKEN}', '{PASSWORD}', '{START}'),
                array($ext_params['subdomain'], $ext_params['token'], $password, $archive_ts),
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
            if (preg_match('|^#EXTINF:.+tvg-id="(?<id>[^"]+)"|', $iValue, $m_id)
                && preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $m3u_lines[$i + 1], $matches)) {
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
}

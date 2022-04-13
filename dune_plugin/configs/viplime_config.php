<?php
require_once 'default_config.php';

class ViplimePluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://cdntv.online/high/%s/playlist.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->EPG_PATH = 'viplime';
        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);

        $ext_params = $channel->get_ext_params();
        if (isset($ext_params['quality'])) {
            $url = str_replace('{QUALITY}', $ext_params['quality'], $url);
        }

        //hd_print("AdjustStreamUrl: $url");

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        if ($this->get_format($plugin_cookies) === 'mpeg') {
            // replace hls to mpegts
            $url = str_replace('.m3u8', '.mpeg', $url);
        }

        // hd_print("Stream url:  " . $url);

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

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

    public function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            $url = sprintf('http://epg.ott-play.com/%s/epg/%s.json', $this->EPG_PATH, $id);
            if (isset($plugin_cookies->use_epg_proxy) && $plugin_cookies->use_epg_proxy === 'yes') {
                $url = str_replace('ott-play.com', 'esalecrm.net', $url);
            }

            return $url;
        }

        return null;
    }
}

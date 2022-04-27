<?php
require_once 'cbilling_vod_impl.php';

class AntifrizPluginConfig extends Cbilling_Vod_Impl
{
    const PLAYLIST_TV_URL = 'http://af-play.com/playlist/%s.m3u8';
    const MEDIA_URL_TEMPLATE_MPEG = 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}';
    const MEDIA_URL_TEMPLATE_ARCHIVE_HLS = 'http://{DOMAIN}/{ID}/archive-{START}-10800.m3u8?token={TOKEN}';
    const MEDIA_URL_TEMPLATE_ARCHIVE_MPEG = 'http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/s/(?<token>.+)/(?<id>.+)/.*$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/s/{TOKEN}/{ID}/video.m3u8');

        $this->set_epg_param('first','epg_root', '');
        $this->set_epg_param('first','epg_url', self::API_HOST .'/epg/{CHANNEL}/?date=');
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

        $ext_params = $channel->get_ext_params();
        $domain = explode(':', $ext_params['subdomain']);
        switch ($this->get_format($plugin_cookies)) {
            case 'hls':
                // http://bethoven.af-stream.com:1600/s/qdfgjyuync/pervyj-hd/video.m3u8
                if ((int)$archive_ts > 0) {
                    // hls archive url completely different, make it from scratch
                    $url = str_replace(
                        array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}'),
                        array($domain[0], $channel->get_channel_id(), $ext_params['token'], $archive_ts),
                        self::MEDIA_URL_TEMPLATE_ARCHIVE_HLS);
                }
                break;
            case 'mpeg':
                // mpeg url also different against hls, make it from scratch
                $url = str_replace(
                    array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}'),
                    array($domain[0], $channel->get_channel_id(), $ext_params['token'], $archive_ts),
                    ((int)$archive_ts > 0) ? self::MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : self::MEDIA_URL_TEMPLATE_MPEG);
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);
        // hd_print("Domain:      " . $subdomain);
        // hd_print("Token:       " . $ext_params['token']);
        // hd_print("Archive TS:  " . $archive_ts);

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        if (!parent::GetAccountInfo($plugin_cookies, $account_data, $force)) {
            return false;
        }

        $plugin_cookies->subdomain_local = $account_data['subdomain'];
        $plugin_cookies->token = $account_data['token'];
        return true;
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

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $password);
            case 'movie':
                return self::API_HOST . '/genres';
        }

        return '';
    }
}

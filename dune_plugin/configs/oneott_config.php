<?php
require_once 'default_config.php';

class OneottPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://list.1ott.net/api/%s/high/ottplay.m3u8';
    const API_HOST = 'http://epg.propg.net';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');

        $this->set_epg_param('epg_url', self::API_HOST . '/{CHANNEL}/epg2/{DATE}', 'first');
        $this->set_epg_param('epg_root', '', 'first');
        $this->set_epg_param('start', 'start', 'first');
        $this->set_epg_param('end', 'stop', 'first');
        $this->set_epg_param('title', 'epg', 'first');
        $this->set_epg_param('description', 'desc', 'first');
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

        if (empty($plugin_cookies->token)) {
            hd_print("User token not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $plugin_cookies->token);
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
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);

        if ($force === false && !empty($plugin_cookies->token)) {
            return true;
        }

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        try {
            $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $login, $password);
            // provider returns token used to download playlist
            $account_data = json_decode(HD::http_get_document($url), true);
            if (isset($account_data['token'])) {
                $plugin_cookies->token = $account_data['token'];
                return true;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }
}

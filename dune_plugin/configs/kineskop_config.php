<?php
require_once 'default_config.php';

class KineskopPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://knkp.in/%s/%s/%s/1';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>[^/]+)/(?<host>[^/]+)/(?<id>[^/]+)/(?<token>[^/]+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{HOST}/{ID}/{TOKEN}.m3u8');

        $this->set_epg_param('first','epg_url','http://epg.esalecrm.net/kineskop/epg/{CHANNEL}.json');
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
            // http://de.kineskop.tv/site2/119/1113391_541b6bc57cc71771_0_0_fs2.m3u8
            $ext_params = $channel->get_ext_params();
            $url = str_replace(array('{DOMAIN}', '{HOST}', '{ID}', '{TOKEN}'),
                array($ext_params['subdomain'], $ext_params['host'], $channel->get_channel_id(), $ext_params['token']),
                $this->get_feature(MEDIA_URL_TEMPLATE_HLS));
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

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
        $server_ops = $this->get_server_opts($plugin_cookies);

        return sprintf(self::PLAYLIST_TV_URL, $login, $password, strtolower($server_ops[$server]));
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return array(
            'DE',
            'PL',
            'US',
            'RU'
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

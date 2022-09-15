<?php
require_once 'default_config.php';

class OnecentPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://only4.tv/pl/%s/102/only4tv.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/index\.m3u8\?token=(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}');
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(SECONDARY_EPG, true);

        $this->set_epg_param('first','epg_url', 'http://epg.iptvx.one/api/id/{CHANNEL}.json');
        $this->set_epg_param('first','epg_root', 'ch_programme');
        $this->set_epg_param('first','epg_start', 'start');
        $this->set_epg_param('first','epg_end', '');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_time_format', 'd-m-Y H:i');
        $this->set_epg_param('first','epg_timezone', 10800); // // iptvx.one uses moscow time (UTC+3)

        $this->set_epg_param('second','epg_url','http://epg.drm-play.ml/iptvx.one/epg/{CHANNEL}.json');
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
            $ext_params = $channel->get_ext_params();
            $url = str_replace(array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token']),
                $this->get_feature(MEDIA_URL_TEMPLATE_HLS));
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

        $password = $this->get_password($plugin_cookies);
        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }
}

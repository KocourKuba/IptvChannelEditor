<?php
require_once 'default_config.php';

class OnecentPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://only4.tv/pl/%s/102/only4tv.m3u8';
    const API_HOST = 'http://technic.cf/epg-iptvxone';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/index\.m3u8\?token=(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('epg_url', self::API_HOST . '/epg_day?id={CHANNEL}&day={DATE}', 'first');
        $this->set_epg_param('epg_root', 'data', 'first');
        $this->set_epg_param('start', 'begin', 'first');
        $this->set_epg_param('end', 'end', 'first');
        $this->set_epg_param('title', 'title', 'first');
        $this->set_epg_param('description', 'description', 'first');
        $this->set_epg_param('date_format', 'Y.m.d', 'first');
        $this->set_epg_param('epg_mapper_url', self::API_HOST . '/channels', 'first');
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
        //hd_print("AdjustStreamUrl: $url");

        switch ($this->get_format($plugin_cookies))
        {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('index.m3u8', "index-$archive_ts-10800.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('index.m3u8', "archive-$archive_ts-10800.ts", $url);
                } else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                break;
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

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }
}

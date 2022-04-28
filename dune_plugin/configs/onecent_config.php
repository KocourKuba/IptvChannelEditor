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

        $this->set_epg_param('first','epg_url', 'http://epg.iptvx.one/api/id/{CHANNEL}.json');
        $this->set_epg_param('first','epg_root', 'ch_programme');
        $this->set_epg_param('first','epg_start', 'start');
        $this->set_epg_param('first','epg_end', '');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_time_format', 'd-m-Y H:i');
        $this->set_epg_param('first','epg_timezone', 'Europe/Moscow');

        $this->set_epg_param('second','epg_url', 'http://technic.cf/epg-iptvxone/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('second','epg_root', 'data');
        $this->set_epg_param('second','epg_start', 'begin');
        $this->set_epg_param('second','epg_end', 'end');
        $this->set_epg_param('second','epg_title', 'title');
        $this->set_epg_param('second','epg_desc', 'description');
        $this->set_epg_param('second','epg_date_format', 'Y.m.d');
        $this->set_epg_param('second','epg_use_mapper', true);
        $this->set_epg_param('second','epg_mapper_url', 'http://technic.cf/epg-iptvxone/channels');
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

        $password = isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;
        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }
}

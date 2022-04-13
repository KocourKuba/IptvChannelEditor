<?php
require_once 'default_config.php';

class IptvonlinePluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://iptv.online/play/%s/m3u8';
    const API_URL = 'http://technic.cf/epg-iptvxone';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>[^/]+)/play/(?<id>[^/]+)/(?<token>[^/]+)/video\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/play/{ID}/{TOKEN}/video.m3u8');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('epg_root', 'data');
        $this->set_epg_param('start', 'begin');
        $this->set_epg_param('end', 'end');
        $this->set_epg_param('title', 'title');
        $this->set_epg_param('description', 'description');
        $this->set_epg_param('date_format', 'Y.m.d');
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

        switch ($this->get_format($plugin_cookies)) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "video-$archive_ts-10800.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "archive-$archive_ts-10800.ts", $url);
                }
                else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * @param $plugin_cookies
     * @param array &$account_data
     * @param bool $force
     * @return bool
     */
    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        if (!parent::GetAccountInfo($plugin_cookies, &$account_data, $force)) {
            return false;
        }

        $mapper = HD::MapTvgID(self::API_URL . '/channels');
        hd_print("TVG ID Mapped: " . count($mapper));
        $this->set_epg_param('tvg_id_mapper', $mapper);

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

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }

    /**
     * @param string $type
     * @param string $id
     * @param int $day_start_ts
     * @param $plugin_cookies
     * @return string|null
     */
    public function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        $params = $this->get_epg_params($type);
        if ($type === 'first') {
            $epg_date = gmdate($params['date_format'], $day_start_ts);
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('%s/epg_day?id=%s&day=%s', self::API_URL, $id, $epg_date); // epg_id date(Y.m.d)
        }

        return null;
    }
}

<?php
require_once 'default_config.php';

class LightiptvPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://lightiptv.cc/playlist/hls/%s.m3u';

    public function __construct()
    {
        parent::__construct();

        static::$EPG_PATH = 'lightiptv';
        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS', 'hls2' => 'HLS2', 'mpeg' => 'MPEG-TS');
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/video\.m3u8\?token=(?<password>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}';
        static::$FEATURES[SQUARE_ICONS] = true;
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = str_replace('{PASSWORD}', $password, $url);

        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "video-$archive_ts-10800.m3u8", $url);
                }
                break;
            case 'hls2':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "video-$archive_ts-10800.m3u8", $url);
                } else {
                    $url = str_replace('video.m3u8', "index.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "timeshift_abs-$archive_ts.ts", $url);
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

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
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
                && preg_match(static::$FEATURES[M3U_STREAM_URL_PATTERN], $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            if (file_exists($this->GET_TMP_STORAGE_PATH())) {
                unlink($this->GET_TMP_STORAGE_PATH());
            }
        }

        return $pl_entries;
    }

    public static function UpdateStreamUrlID($channel_id, $ext_params)
    {
        return str_replace('{TOKEN}', $ext_params['token'], static::$FEATURES[MEDIA_URL_TEMPLATE_HLS]);
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            $url = sprintf('http://epg.ott-play.com/%s/epg/%s.json', static::$EPG_PATH, $id);
            if (isset($plugin_cookies->use_epg_proxy) && $plugin_cookies->use_epg_proxy === 'yes') {
                $url = str_replace('ott-play.com', 'esalecrm.net', $url);
            }

            return $url;
        }

        return null;
    }
}

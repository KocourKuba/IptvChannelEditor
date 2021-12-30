﻿<?php
require_once 'default_config.php';

class LightiptvPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://lightiptv.cc/playlist/hls/%s.m3u';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[MPEG_TS_SUPPORTED] = true;
        static::$FEATURES[HLS2_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/video\.m3u8\?token=(?<password>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{TOKEN}/video.m3u8?token={PASSWORD}';
        static::$FEATURES[SQUARE_ICONS] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_template'] = 'http://epg.ott-play.com/lightiptv/epg/%s.json'; // epg_id
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
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $m3u_lines = self::FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if (preg_match('|^#EXTINF:.+tvg-id="(?<id>[^"]+)"|', $iValue, $m_id)
                && preg_match(static::$FEATURES[M3U_STREAM_URL_PATTERN], $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            throw new DuneException(
                'Empty provider playlist', 0,
                ActionFactory::show_error(
                    true,
                    'Ошибка скачивания плейлиста',
                    array(
                        'Пустой плейлист провайдера!',
                        'Проверьте подписку или подключение к Интернет.')));
        }

        return $pl_entries;
    }

    public static function UpdateStreamUrlID($channel_id, $ext_params)
    {
        return str_replace('{TOKEN}', $ext_params['token'], static::$FEATURES[MEDIA_URL_TEMPLATE_HLS]);
    }
}

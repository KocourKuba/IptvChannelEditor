﻿<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/(?<id>.+)/(?:.*)\?token=(?<token>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}';

        static::$EPG_PARSER_PARAMS['second']['date_format'] = 'Ymd';
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
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = self::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('index.m3u8', 'mpegts', $url);
        }

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

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        $epg_date = gmdate(static::$EPG_PARSER_PARAMS[$type]['date_format'], $day_start_ts);
        hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
        switch ($type) {
            case 'first':
                return sprintf('http://api.program.spr24.net/api/program?epg=%s&date=%s', $id, $epg_date); // epg_id date(Y-m-d)
            case 'second':
                return sprintf('http://epg.arlekino.tv/api/program?epg=%s&date=%s', $id, $epg_date); // epg_id date(Ymd)
        }

        return null;
    }
}

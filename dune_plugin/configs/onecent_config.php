﻿<?php
require_once 'default_config.php';

class OnecentPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://only4.tv/pl/%s/102/only4tv.m3u8';
    const API_URL = 'http://technic.cf/epg-soveni';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/(?<id>.+)/index\.m3u8\?token=(?<token>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}';
        static::$FEATURES[SQUARE_ICONS] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'data';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'begin';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'end';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'description';
        static::$EPG_PARSER_PARAMS['first']['date_format'] = 'Y.m.d';
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
        //hd_print("AdjustStreamUrl: $url");

        switch (self::get_format($plugin_cookies))
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

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        if (!parent::GetAccountInfo($plugin_cookies, &$account_data, $force)) {
            return false;
        }

        try {
            $content = HD::http_get_document(self::API_URL . '/channels');
            // stripe UTF8 BOM if exists
            $json_data = json_decode(ltrim($content, "\xEF\xBB\xBF"), true);
            static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper'] = $json_data['data'];
            hd_print("TVG ID Mapped: " . count(static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper']));
        } catch (Exception $ex) {
            return false;
        }

        return true;
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            $epg_date = gmdate(static::$EPG_PARSER_PARAMS['first']['date_format'], $day_start_ts);
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('%s/epg_day?id=%s&day=%s', self::API_URL, $id, $epg_date); // epg_id date(Y.m.d)
        }

        return null;
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
}

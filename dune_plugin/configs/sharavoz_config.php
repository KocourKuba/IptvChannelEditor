﻿<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>.+)/(?:.*)\?token=(?<token>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/index.m3u8?token={TOKEN}');

        $this->set_epg_param('date_format', 'Ymd', 'second');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        if ($this->get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('index.m3u8', 'mpegts', $url);
        }

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

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

    public function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        hd_print("Fetching EPG for ID: '$id'");
        switch ($type) {
            case 'first':
                return sprintf('http://api.program.spr24.net/api/program?epg=%s', $id);
            case 'second':
                return sprintf('http://epg.arlekino.tv/api/program?epg=%s', $id);
        }

        return null;
    }
}

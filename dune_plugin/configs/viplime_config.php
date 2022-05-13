﻿<?php
require_once 'default_config.php';

class ViplimePluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://cdntv.online/high/%s/playlist.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<quality>.+)/(?<token>.+)/(?<id>.+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.m3u8');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{QUALITY}/{TOKEN}/{ID}.mpeg');

        $this->set_epg_param('first','epg_url','http://epg.esalecrm.net/viplime/epg/{CHANNEL}.json');
        //$this->set_epg_param('first','epg_url','http://epg.esalecrm.net/viplime/epg/{CHANNEL}.json');
        //$this->set_epg_param('first','epg_use_hash', true);
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
            $template = $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}', 'QUALITY'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token'], $ext_params['quality']),
                $template);
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

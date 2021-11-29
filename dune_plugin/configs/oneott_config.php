﻿<?php
require_once 'default_config.php';

class OneottPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'LOGIN';
    public static $MPEG_TS_SUPPORTED = true;

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.propg.net/%s/epg2/%s'; // epg_id date(YYYY-MM-DD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/1ott/epg/%s.json'; // epg_id

    public function __construct()
    {
        parent::__construct();
        static::$EPG_PARSER_PARAMS['first']['epg_root'] = '';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'start';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'stop';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'epg';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'desc';
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
            $url = str_replace('/hls/pl.m3u8', '', $url);
            $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    protected static function GetTemplatedUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $ott_key = $plugin_cookies->ott_key_local;
        if (empty($ott_key)) {
            $ott_key = $plugin_cookies->ott_key;
        }

        if (empty($ott_key)) {
            hd_print("User token not set");
        }

        return sprintf('http://list.1ott.net/api/%s/high/ottplay.m3u8', $ott_key);
    }

    /**
     * Get information from the account
     * @param $plugin_cookies
     * @param &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        if (empty($plugin_cookies->login) && empty($plugin_cookies->password)) {
            hd_print("Login or password not set");
            return false;
        }

        try {
            $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $plugin_cookies->login, $plugin_cookies->password);
            // provider returns token used to download playlist
            $account_data = json_decode(HD::http_get_document($url), true);
            if (isset($account_data['token'])) {
                $plugin_cookies->ott_key = $account_data['token'];
                return true;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }
}

<?php
require_once 'default_config.php';

class OneottPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://list.1ott.net/api/%s/high/ottplay.m3u8';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8';

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
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('/hls/pl.m3u8', '', $url);
        }

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        if (empty($plugin_cookies->token)) {
            hd_print("User token not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $plugin_cookies->token);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);

        if ($force === false && !empty($plugin_cookies->token)) {
            return true;
        }

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        try {
            $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $login, $password);
            // provider returns token used to download playlist
            $account_data = json_decode(HD::http_get_document($url), true);
            if (isset($account_data['token'])) {
                $plugin_cookies->token = $account_data['token'];
                return true;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        switch($type)
        {
            case 'first':
                hd_print("Fetching EPG for ID: '$id'");
                return sprintf('http://epg.ott-play.com/1ott/epg/%s.json', $id);
            case 'second':
                $epg_date = gmdate(static::$EPG_PARSER_PARAMS['second']['date_format'], $day_start_ts);
                hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
                return sprintf('http://epg.propg.net/%s/epg2/%s', $id, $epg_date); // epg_id date(Y-m-d)
        }

        return null;
    }
}

<?php
require_once 'default_config.php';

class VidokPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://bddpv.plist.top/p/%s';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[BALANCE_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/p/(?<token>.+)/(?<id>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/p/{TOKEN}/{ID}';

        static::$EPG_PARSER_PARAMS['first']['config'] = $this;
        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'epg';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'start';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'end';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'description';
        static::$EPG_PARSER_PARAMS['first']['date_format'] = 'dmy';
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

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        if (empty($plugin_cookies->ott_key)) {
            hd_print("User token not set");
        }

        return sprintf(self::PLAYLIST_TV_URL, $plugin_cookies->ott_key);
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
        if ($force === false && !empty($plugin_cookies->ott_key)) {
            return true;
        }

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        $plugin_cookies->ott_key = md5(strtolower($login) . md5($password));

        try {
            $url = 'http://sapi.ott.st/v2.4/json/account?token=' . $plugin_cookies->ott_key;
            // provider returns token used to download playlist
            $account_data = json_decode(HD::http_get_document($url), true);
            if (isset($account_data['account']['login'])) {
                return true;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            $epg_date = gmdate(static::$EPG_PARSER_PARAMS[$type]['date_format'], $day_start_ts);
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('http://sapi.ott.st/v2.4/json/epg?cid=%s&day=%s&token=%s', $id, $epg_date, $plugin_cookies->ott_key); // epg_id date(Ymd)
        }

        return null;
    }
}

<?php
require_once 'default_config.php';

class VidokPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://vidok.tv/p/%s';
    const API_HOST = 'http://sapi.ott.st/v2.4/json/';

    protected static $settings;

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS');
        static::$FEATURES[BALANCE_SUPPORTED] = true;
        static::$FEATURES[QUALITY_SUPPORTED] = true;
        static::$FEATURES[SERVER_SUPPORTED] = true;
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

        if (empty($plugin_cookies->token)) {
            hd_print("User token not set");
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
    public static function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        hd_print("GetAccountInfo");
        if (!self::ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        if ($force === false) {
            return true;
        }

        try {
            $url = self::API_HOST . "account?token=$plugin_cookies->token";
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

    public static function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = self::GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            ControlFactory::add_label($defs, 'Ошибка!', $text[0]);
            ControlFactory::add_label($defs, 'Описание:', $text[1]);
            return;
        }

        ControlFactory::add_label($defs, 'Баланс:', $account_data['account']['balance'] . ' €');
        ControlFactory::add_label($defs, 'Логин:', $account_data['account']['login']);
        $packages = $account_data['account']['packages'];
        if (count($packages) === 0) {
            ControlFactory::add_label($defs, 'Пакеты: ', 'Нет пакетов');
            return;
        }

        foreach ($packages as $item) {
            ControlFactory::add_label($defs, 'Пакет:', $item['name'] .' до '. date('j.m.Y', $item['expire']));
            ControlFactory::add_vgap($defs, -10);
        }
        ControlFactory::add_vgap($defs, 20);
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            return sprintf(self::API_HOST . 'epg2?cid=%s&token=%s', $id, $plugin_cookies->token);
        }

        return null;
    }

    public static function get_server_opts($plugin_cookies)
    {
        if (self::load_settings($plugin_cookies)) {
            $ops = array();
            foreach (self::$settings['settings']['lists']['servers'] as $item) {
                $ops[$item['id']] = $item['name'];
            }
            return $ops;
        }

        return array();
    }

    public static function get_server($plugin_cookies)
    {
        if (self::load_settings($plugin_cookies)) {
            return self::$settings['settings']['current']['server_id'];
        }

        return null;
    }

    public static function set_server($plugin_cookies)
    {
        return self::save_settings($plugin_cookies, 'server');
    }

    public static function get_quality_opts($plugin_cookies)
    {
        if (self::load_settings($plugin_cookies)) {
            $ops = array();
            foreach (self::$settings['settings']['lists']['quality'] as $item) {
                $ops[$item['id']] = $item['name'];
            }
            return $ops;
        }

        return array();
    }

    public static function get_quality($plugin_cookies)
    {
        if (self::load_settings($plugin_cookies))
        {
            return self::$settings['settings']['current']['quality'];
        }
        return null;
    }

    public static function set_quality($plugin_cookies)
    {
        return self::save_settings($plugin_cookies, 'quality');
    }

    protected static function load_settings(&$plugin_cookies)
    {
        if (!self::ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        if (empty(self::$settings)) {
            try {
                $url = self::API_HOST . "settings?token=$plugin_cookies->token";
                // provider returns token used to download playlist
                self::$settings = json_decode(HD::http_get_document($url), true);
            } catch (Exception $ex) {
                hd_print("Settings not loaded");
            }
        }

        return !empty(self::$settings);
    }

    protected static function save_settings(&$plugin_cookies, $param)
    {
        hd_print("save settings $param to {$plugin_cookies->$param}");

        if (!self::ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        try {
            $url = self::API_HOST . "settings_set?$param={$plugin_cookies->$param}&token=$plugin_cookies->token";
            HD::http_get_document($url);
            return true;
        } catch (Exception $ex) {
            hd_print("Settings not saved");
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////

    protected static function ensure_token_loaded(&$plugin_cookies)
    {
        if (!empty($plugin_cookies->token)) {
            return true;
        }

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return false;
        }

        $plugin_cookies->token = md5(strtolower($login) . md5($password));

        return !empty($plugin_cookies->token);
    }
}

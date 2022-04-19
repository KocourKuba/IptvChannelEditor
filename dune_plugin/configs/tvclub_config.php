<?php
require_once 'default_config.php';

class TvclubPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://celn.shott.top/p/%s';
    const API_HOST = 'http://api.iptv.so/0.9/json';

    protected static $settings;

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('mpeg' => 'MPEG-TS'));
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/p/(?<token>.+)/(?<id>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/p/{TOKEN}/{ID}');

        $this->set_epg_param('first','epg_url', self::API_HOST . '/epg?token={TOKEN}&channels={CHANNEL}&time={TIME}&period=24');
        $this->set_epg_param('first','epg_root', 'epg|channels|0|epg');
        $this->set_epg_param('first','epg_start', 'start');
        $this->set_epg_param('first','epg_end', 'end');
        $this->set_epg_param('first','epg_title', 'text');
        $this->set_epg_param('first','epg_desc', 'description');
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
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        // hd_print("Stream url:  " . $url);

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
        if (!self::ensure_token_loaded($plugin_cookies)) {
            hd_print("No token!");
            return '';
        }

        if (empty($plugin_cookies->token)) {
            hd_print("User token not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $plugin_cookies->token);
    }

    /**
     * @param string $url
     * @param int $archive_ts
     * @return string
     */
    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "utc=$archive_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
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

        $account_data = self::$settings;
        if ($force !== false && self::load_settings($plugin_cookies)) {
            $account_data = self::$settings;
        }

        return isset($account_data['account']['info']['login']);
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = $this->GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            Control_Factory::add_label($defs, 'Ошибка!', $text[0], -10);
            Control_Factory::add_label($defs, 'Описание:', $text[1], 20);
            return;
        }

        Control_Factory::add_label($defs, 'Баланс:', $account_data['account']['info']['balance'] . ' €', -10);
        Control_Factory::add_label($defs, 'Логин:', $account_data['account']['info']['login'], -10);
        Control_Factory::add_label($defs, 'Сервер:', $account_data['account']['settings']['server_name'], -10);
        Control_Factory::add_label($defs, 'Часовой пояс:', $account_data['account']['settings']['tz_name'] . " {$account_data['account']['settings']['tz_gmt']}", -10);

        $packages = $account_data['account']['services'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, 'Пакеты: ', 'Нет пакетов', 20);
            return;
        }

        foreach ($packages as $item) {
            Control_Factory::add_label($defs, 'Пакет:', $item['name'] . ' ' . $item['type'] .' до '. date('j.m.Y', $item['expire']), -10);
        }

        Control_Factory::add_vgap($defs, 20);
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    public function get_format($plugin_cookies)
    {
        return isset($plugin_cookies->format) ? $plugin_cookies->format : 'mpeg';
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_server_opts($plugin_cookies)
    {
        try {
            $url = self::API_HOST . "/servers?token=$plugin_cookies->token";
            $servers = json_decode(HD::http_get_document($url), true);
            $ops = array();
            foreach ($servers['servers'] as $item) {
                $ops[$item['id']] = $item['name'];
            }
            return $ops;
        } catch (Exception $ex) {
            hd_print("Servers not loaded");
        }

        return array();
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_server($plugin_cookies)
    {
        if (self::load_settings($plugin_cookies)) {
            return self::$settings['account']['settings']['server_id'];
        }

        return null;
    }

    /**
     * @param $plugin_cookies
     */
    public function set_server($plugin_cookies)
    {
        self::save_settings($plugin_cookies, 'server');
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected static function load_settings(&$plugin_cookies)
    {
        if (!self::ensure_token_loaded($plugin_cookies)) {
            hd_print("No token!");
            return false;
        }

        try {
            $url = self::API_HOST . "/account?token=$plugin_cookies->token";
            // provider returns token used to download playlist
            self::$settings = json_decode(HD::http_get_document($url), true);
        } catch (Exception $ex) {
            hd_print("Settings not loaded");
        }

        return !empty(self::$settings);
    }

    /**
     * @param $plugin_cookies
     * @param string $param
     * @return bool
     */
    protected static function save_settings(&$plugin_cookies, $param)
    {
        hd_print("save settings $param to {$plugin_cookies->$param}");

        if (!self::ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        try {
            $url = self::API_HOST . "/set?token=$plugin_cookies->token&$param={$plugin_cookies->$param}";
            HD::http_get_document($url);
            self::load_settings($plugin_cookies);
            return true;
        } catch (Exception $ex) {
            hd_print("Settings not saved");
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////

    /**
     * @param $plugin_cookies
     * @return bool
     */
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

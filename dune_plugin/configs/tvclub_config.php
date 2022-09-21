<?php
require_once 'default_config.php';

class TvclubPluginConfig extends Default_Config
{
    const API_HOST = 'http://api.iptv.so/0.9/json';

    protected static $settings = array();

    public function load_config()
    {
        parent::load_config();

        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://celn.shott.top/p/{TOKEN}');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/p/(?<token>.+)/(?<id>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'append');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/p/{TOKEN}/{ID}');

        $this->set_epg_param(EPG_FIRST,EPG_URL, self::API_HOST . '/epg?token={TOKEN}&channels={ID}&time={TIMESTAMP}&period=24');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, 'epg|channels|0|epg');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'start');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'end');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'text');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'description');
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account");

        $account_data = self::$settings;
        if ($force !== false && $this->load_settings($plugin_cookies)) {
            $account_data = self::$settings;
        }

        if (!isset($account_data['account']['info']['login'])) {
            return false;
        }
        return $account_data;
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = $this->GetAccountInfo($plugin_cookies, true);
        if ($account_data === false) {
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
        return isset($plugin_cookies->format) ? $plugin_cookies->format : MPEG;
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_server_opts($plugin_cookies)
    {
        try {
            $url = self::API_HOST . "/servers?token=$plugin_cookies->token";
            $servers = HD::DownloadJson($url);
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
    public function get_server_id($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies)) {
            return self::$settings['account']['settings']['server_id'];
        }

        return null;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
        $plugin_cookies->server = $server;
        $this->save_settings($plugin_cookies, 'server');
    }

    /**
     * @param $plugin_cookies
     * @return bool
     */
    protected function load_settings(&$plugin_cookies)
    {
        if (!$this->ensure_token_loaded($plugin_cookies)) {
            hd_print("No token!");
            return false;
        }

        try {
            $url = self::API_HOST . "/account?token=$plugin_cookies->token";
            // provider returns token used to download playlist
            self::$settings = HD::DownloadJson($url);
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
    protected function save_settings(&$plugin_cookies, $param)
    {
        hd_print("save settings $param to {$plugin_cookies->$param}");

        if (!$this->ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        try {
            $url = self::API_HOST . "/set?token=$plugin_cookies->token&$param={$plugin_cookies->$param}";
            HD::http_get_document($url);
            $this->load_settings($plugin_cookies);
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
    protected function ensure_token_loaded(&$plugin_cookies)
    {
        if (!empty($plugin_cookies->token)) {
            return true;
        }

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return false;
        }

        $plugin_cookies->token = md5($login . md5($password));

        return !empty($plugin_cookies->token);
    }
}

<?php
require_once 'lib/default_config.php';

class tvclub_config extends default_config
{
    const API_HOST = 'http://api.iptv.so/0.9/json';

    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(BALANCE_SUPPORTED, true);
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
    public function get_servers($plugin_cookies)
    {
        $servers = parent::get_servers($plugin_cookies);
        if (empty($servers)) {
            try {
                $json = HD::DownloadJson(self::API_HOST . "/servers?token=$plugin_cookies->token");
                $servers = array();
                foreach ($json['servers'] as $item) {
                    $servers[$item['id']] = $item['name'];
                }
                $this->set_servers($servers);
            } catch (Exception $ex) {
                hd_print("Servers not loaded");
            }
        }

        return $servers;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server_id($server, $plugin_cookies)
    {
        $this->save_settings($plugin_cookies, 'server');
        parent::set_server_id($server, $plugin_cookies);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account: $force");

        if ($force !== false || empty($this->account_data)) {
            if (!$this->load_settings($plugin_cookies)) {
                return false;
            }
        }

        return $this->account_data;
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        if ($this->GetAccountInfo($plugin_cookies, true) === false) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            Control_Factory::add_label($defs, 'Ошибка!', $text[0], -10);
            Control_Factory::add_label($defs, 'Описание:', $text[1], 20);
            return;
        }

        $info = $this->account_data['account']['info'];
        $settings = $this->account_data['account']['settings'];
        Control_Factory::add_label($defs, 'Баланс:', $info['balance'] . ' €', -10);
        Control_Factory::add_label($defs, 'Логин:', $info['login'], -10);
        Control_Factory::add_label($defs, 'Сервер:', $settings['server_name'], -10);
        Control_Factory::add_label($defs, 'Часовой пояс:', $settings['tz_name'] . " {$settings['tz_gmt']}", -10);

        $packages = $this->account_data['account']['services'];
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
     * @return bool
     */
    protected function load_settings(&$plugin_cookies)
    {
        try {
            if (!$this->ensure_token_loaded($plugin_cookies)) {
                throw new Exception("Token not loaded");
            }

            $url = self::API_HOST . "/account?token=$plugin_cookies->token";
            // provider returns token used to download playlist
            $json = HD::DownloadJson($url);
            if (!isset($json['account']['info']['login'])) {
                throw new Exception("Account status unknown");
            }
            $this->account_data = $json;
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return false;
        }

        return !empty($this->account_data);
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
        if (empty($plugin_cookies->token)) {
            $login = $this->get_login($plugin_cookies);
            $password = $this->get_password($plugin_cookies);

            if (empty($login) || empty($password)) {
                hd_print("Login or password not set");
                return false;
            }

            $plugin_cookies->token = md5($login . md5($password));
        }

        return true;
    }
}

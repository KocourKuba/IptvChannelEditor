﻿<?php
require_once 'lib/default_config.php';

class vidok_config extends default_config
{
    const API_HOST = 'http://sapi.ott.st/v2.4/json';

    protected static $settings = array();

    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(QUALITY_OPTIONS, true);
        $this->set_feature(SERVER_OPTIONS, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://vidok.tv/p/{TOKEN}');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>.+)/p/(?<token>.+)/(?<id>.+)$|');

        $this->set_stream_param(HLS,CU_TYPE, 'append');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/p/{TOKEN}/{ID}');

        $this->set_epg_param(EPG_FIRST,EPG_URL, self::API_HOST . '/epg2?cid={ID}&token={TOKEN}');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, 'epg');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'tart');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'end');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'title');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'description');
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_servers($plugin_cookies)
    {
        $servers = parent::get_servers($plugin_cookies);
        if (empty($servers) && $this->load_settings($plugin_cookies)) {
            $servers = array();
            foreach (self::$settings['settings']['lists']['servers'] as $item) {
                $servers[$item['id']] = $item['name'];
            }
        }

        return $servers;
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_server_id($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies)) {
            $server = self::$settings['settings']['current']['server']['id'];
        } else {
            $server = parent::get_server_id($plugin_cookies);
        }

        return $server;
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
     * @param $plugin_cookies
     * @return array
     */
    public function get_qualities($plugin_cookies)
    {
        $quality = parent::get_qualities($plugin_cookies);
        if (empty($quality) && $this->load_settings($plugin_cookies)) {
            $quality = array();
            foreach (self::$settings['settings']['lists']['quality'] as $item) {
                $quality[$item['id']] = $item['name'];
            }
        }

        return $quality;
    }

    /**
     * @param $quality
     * @param $plugin_cookies
     */
    public function set_quality_id($quality, $plugin_cookies)
    {
        $this->save_settings($plugin_cookies, 'quality');
        parent::set_quality_id($quality, $plugin_cookies);
    }

    /**
     * @param $plugin_cookies
     * return bool
     */
    protected function load_settings(&$plugin_cookies)
    {
        if (!$this->ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        if (empty(self::$settings)) {
            try {
                $url = self::API_HOST . "/settings?token=$plugin_cookies->token";
                // provider returns token used to download playlist
                self::$settings = HD::DownloadJson($url);
                hd_print(json_encode(self::$settings));
            } catch (Exception $ex) {
                hd_print("Settings not loaded");
            }
        }

        return !empty(self::$settings);
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

        if (!$this->ensure_token_loaded($plugin_cookies)) {
            return false;
        }

        if ($force === false) {
            return array();
        }

        try {
            $url = self::API_HOST . "/account?token=$plugin_cookies->token";
            // provider returns token used to download playlist
            $account_data = HD::DownloadJson($url);
            if (isset($account_data['account']['login'])) {
                self::$settings = $account_data;
                return $account_data;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }

    /**
     * @param array &$defs
     * @param $plugin_cookies
     */
    public function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = $this->GetAccountInfo($plugin_cookies,true);
        if ($account_data === false) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            Control_Factory::add_label($defs, 'Ошибка!', $text[0], -10);
            Control_Factory::add_label($defs, 'Описание:', $text[1], 20);
            return;
        }

        Control_Factory::add_label($defs, 'Баланс:', $account_data['account']['balance'] . ' €', -10);
        Control_Factory::add_label($defs, 'Логин:', $account_data['account']['login'], -10);
        $packages = $account_data['account']['packages'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, 'Пакеты: ', 'Нет пакетов', 20);
            return;
        }

        foreach ($packages as $item) {
            Control_Factory::add_label($defs, 'Пакет:', $item['name'] .' до '. date('j.m.Y', $item['expire']), -10);
        }

        Control_Factory::add_vgap($defs, 20);
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
            $url = self::API_HOST . "/settings_set?$param={$plugin_cookies->$param}&token=$plugin_cookies->token";
            HD::http_get_document($url);
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

        $plugin_cookies->token = md5(strtolower($login) . md5($password));

        return !empty($plugin_cookies->token);
    }
}

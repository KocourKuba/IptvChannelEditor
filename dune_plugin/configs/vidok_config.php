﻿<?php
require_once 'default_config.php';

class VidokPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://vidok.tv/p/%s';
    const API_HOST = 'http://sapi.ott.st/v2.4/json';

    protected static $settings = array();

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(BALANCE_SUPPORTED, true);
        $this->set_feature(QUALITY_SUPPORTED, true);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/p/(?<token>.+)/(?<id>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/p/{TOKEN}/{ID}');

        $this->set_epg_param('first','epg_url', self::API_HOST . '/epg2?cid={CHANNEL}&token={TOKEN}');
        $this->set_epg_param('first','epg_root', 'epg');
        $this->set_epg_param('first','epg_start', 'tart');
        $this->set_epg_param('first','epg_end', 'end');
        $this->set_epg_param('first','epg_title', 'title');
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
        $url = $channel->get_streaming_url();
        if (empty($url)) {
            $template = $this->get_feature(MEDIA_URL_TEMPLATE_HLS);
            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}'),
                array($ext_params['subdomain'], $channel->get_channel_id(), $ext_params['token']),
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

        if (empty($plugin_cookies->token)) {
            hd_print("User token not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $plugin_cookies->token);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account " . $this->PLUGIN_SHOW_NAME);

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
     * @return array
     */
    public function get_server_opts($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies)) {
            $ops = array();
            foreach (self::$settings['settings']['lists']['servers'] as $item) {
                $ops[$item['id']] = $item['name'];
            }
            return $ops;
        }

        return array();
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_server($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies)) {
            return self::$settings['settings']['current']['server_id'];
        }

        return null;
    }

    /**
     * @param $server
     * @param $plugin_cookies
     */
    public function set_server($server, $plugin_cookies)
    {
        $plugin_cookies->server = $server;
        $this->save_settings($plugin_cookies, 'server');
    }

    /**
     * @param $plugin_cookies
     * @return array
     */
    public function get_quality_opts($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies)) {
            $ops = array();
            foreach (self::$settings['settings']['lists']['quality'] as $item) {
                $ops[$item['id']] = $item['name'];
            }
            return $ops;
        }

        return array();
    }

    /**
     * @param $plugin_cookies
     * @return mixed|null
     */
    public function get_quality($plugin_cookies)
    {
        if ($this->load_settings($plugin_cookies))
        {
            return self::$settings['settings']['current']['quality'];
        }
        return null;
    }

    /**
     * @param $plugin_cookies
     */
    public function set_quality($plugin_cookies)
    {
        $this->save_settings($plugin_cookies, 'quality');
    }

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
            } catch (Exception $ex) {
                hd_print("Settings not loaded");
            }
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

<?php
require_once 'cbilling_vod_impl.php';

class cbilling_config extends Cbilling_Vod_Impl
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://247on.cc/playlist/{PASSWORD}_otp_dev{DEVICE_ID}.m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/s/(?<token>.+)/(?<id>.+)\.m3u8$');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{CU_DURATION}.ts?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_URL, self::API_HOST .'/epg/{ID}/?date=');
        $this->set_devices(array('1', '2', '3'));
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        $account_data = parent::GetAccountInfo($plugin_cookies, $force);
        if ($account_data === false) {
            return false;
        }

        $this->account_data = $account_data;

        // this account has special API to get account info
        $password = $this->get_password($plugin_cookies);
        if ($force === false && !empty($password)) {
            return $account_data;
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $headers[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
            return HD::DownloadJson(self::API_HOST . '/auth/info', true, $headers);
        } catch (Exception $ex) {
            return false;
        }
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
            Control_Factory::add_label($defs, 'Описание:', $text[1], -10);
            return;
        }

        Control_Factory::add_label($defs, 'Пакеты: ', empty($account_data['data']['package']) ? 'Нет пакетов' : $account_data['data']['package'], -10);
        Control_Factory::add_label($defs, 'Дата окончания', $account_data['data']['end_date'], -10);
        Control_Factory::add_label($defs, 'Устройств', $account_data['data']['devices_num'], -10);
        Control_Factory::add_label($defs, 'Сервер', $account_data['data']['server'], 20);
    }
}

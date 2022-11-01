<?php
require_once 'cbilling_vod_impl.php';

class cbilling_config extends Cbilling_Vod_Impl
{
    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(Plugin_Constants::BALANCE_SUPPORTED, true);
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
        // this account has special API to get account info
        // {
        //    "data": {
        //        "public_token": "2f5b87bd565ca1234e2sba3fd3919bac",
        //        "private_token": "5acf87d3206a905b83419224703bf666",
        //        "end_time": 1705697968,
        //        "end_date": "2024-01-19 23:59",
        //        "devices_num": 1,
        //        "package": "IPTV HD+SD (позапросный тариф)",
        //        "server": "s01.wsbof.com",
        //        "server_country": "Германия",
        //        "vod": 1,
        //        "ssl": 0,
        //        "disable_adult": 0
        //    }
        // }

        $password = $this->get_password($plugin_cookies);
        try {
            if (empty($password)) {
                throw new Exception("Password not set");
            }

            if ($force !== false || empty($this->account_data)) {
                $headers[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
                $json = HD::DownloadJson(self::API_HOST . '/auth/info', true, $headers);
                if (!isset($json['data'])) {
                    throw new Exception("Account info not loaded");
                }
                $this->account_data = $json['data'];
            }
        } catch (Exception $ex) {
            hd_print($ex->getMessage());
            return false;
        }

        //foreach($this->account_data as $key => $value) hd_print("  $key => $value");
        return $this->account_data;
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

        Control_Factory::add_label($defs, 'Пакеты: ', empty($account_data['package']) ? 'Нет пакетов' : $account_data['package'], -10);
        Control_Factory::add_label($defs, 'Дата окончания', $account_data['end_date'], -10);
        Control_Factory::add_label($defs, 'Устройств', $account_data['devices_num'], -10);
        Control_Factory::add_label($defs, 'Сервер', $account_data['server_country'], -10);
        Control_Factory::add_label($defs, 'Адрес сервера', $account_data['server'], 20);
    }
}

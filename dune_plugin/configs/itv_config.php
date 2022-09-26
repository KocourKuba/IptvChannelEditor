<?php
require_once 'lib/default_config.php';

class itv_config extends default_config
{
    const API_HOST = 'http://api.itv.live';

    public function init_defaults($short_name)
    {
        parent::init_defaults($short_name);

        $this->set_feature(BALANCE_SUPPORTED, true);
    }

    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://itv.ooo/p/{PASSWORD}/hls.m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/(?<id>.+)/[^\?]+\?token=(?<token>.+)$');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts');

        $this->set_epg_param(EPG_FIRST,EPG_URL, self::API_HOST . '/epg/{ID}');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, 'res');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'startTime');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'stopTime');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'title');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'desc');
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

        // this account has special API to get account info
        $password = $this->get_password($plugin_cookies);
        if ($force === false && !empty($password)) {
            return array();
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $url = sprintf(self::API_HOST . '/data/%s', $password);
            $account_data = HD::DownloadJson($url);
            if (empty($account_data['package_info'])) {
                return false;
            }
        } catch (Exception $ex) {
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

        $title = 'Пакеты: ';

        Control_Factory::add_label($defs, 'Баланс:', $account_data['user_info']['cash'] . ' $', -10);
        $packages = $account_data['package_info'];
        if (count($packages) === 0) {
            Control_Factory::add_label($defs, $title, 'Нет пакетов', 20);
            return;
        }

        $list = array();
        foreach ($packages as $item) {
            $list[] = $item['name'];
        }

        $emptyTitle = str_repeat(' ', strlen($title));
        $list_collected = array();
        $isFirstLabel = true;
        foreach($list as $item) {
            $list_collected[] = $item;
            $collected = implode(', ', $list_collected);
            if (strlen($collected) < 30) {
                continue;
            }

            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected, -10);

            if ($isFirstLabel) {
                $isFirstLabel = false;
            }

            $list_collected = array();
        }

        if (count($list_collected) !== 0) {
            Control_Factory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
        }

        Control_Factory::add_vgap($defs, 20);
    }
}

<?php
require_once 'cbilling_vod_impl.php';

class CbillingPluginConfig extends Cbilling_Vod_Impl
{
    const PLAYLIST_TV_URL = 'http://247on.cc/playlist/%s_otp_dev%s.m3u8';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'PIN');
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/s/(?<token>.+)/(?<id>.+)\.m3u8$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/s/{TOKEN}/{ID}.m3u8?utc={START}&lutc={NOW}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{ID}/archive-{START}-10800.ts?token={TOKEN}');
        $this->set_feature(DEVICE_OPTIONS, array('1' => '1', '2' => '2', '3' => '3'));
        $this->set_feature(BALANCE_SUPPORTED, true);

        $this->set_epg_param('first','epg_root', '');
        $this->set_epg_param('first','epg_url', self::API_HOST .'/epg/{CHANNEL}/?date=');
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
        if (!empty($url)) {
            $url = ((int)$archive_ts <= 0) ?: static::UpdateArchiveUrlParams($url, $archive_ts);
        } else {
            $ext_params = $channel->get_ext_params();
            $domain = explode(':', $ext_params['subdomain']);

            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $template = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_HLS : MEDIA_URL_TEMPLATE_HLS);
                    if (((int)$archive_ts <= 0))
                        $domain[0] = $ext_params['subdomain'];
                    break;
                case 'mpeg':
                    $template = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $url = str_replace(
                array('{DOMAIN}', '{ID}', '{TOKEN}', '{START}', '{NOW}'),
                array($domain[0], $channel->get_channel_id(), $ext_params['token'], $archive_ts, time()),
                $template);
        }

        // hd_print("Stream url:  $url");

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
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
        if (!parent::GetAccountInfo($plugin_cookies, $account_data, $force)) {
            return false;
        }

        $this->account_data = $account_data;

        // this account has special API to get account info
        $password = isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;
        if ($force === false && !empty($password)) {
            return true;
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $headers[CURLOPT_HTTPHEADER] = array("accept: */*", "x-public-key: $password");
            $content = HD::http_get_document(self::API_HOST . '/auth/info', $headers);
        } catch (Exception $ex) {
            return false;
        }

        // stripe UTF8 BOM if exists
        $account_data = json_decode(ltrim($content, "\xEF\xBB\xBF"), true);
        return true;
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
            Control_Factory::add_label($defs, 'Описание:', $text[1], -10);
            return;
        }

        Control_Factory::add_label($defs, 'Пакеты: ', empty($account_data['data']['package']) ? 'Нет пакетов' : $account_data['data']['package'], -10);
        Control_Factory::add_label($defs, 'Дата окончания', $account_data['data']['end_date'], -10);
        Control_Factory::add_label($defs, 'Устройств', $account_data['data']['devices_num'], -10);
        Control_Factory::add_label($defs, 'Сервер', $account_data['data']['server'], 20);
    }

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;

        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $password, isset($plugin_cookies->device_number) ? $plugin_cookies->device_number : '1');
            case 'movie':
                return self::API_HOST . '/genres';
        }

        return '';
    }

    /**
     * @param $plugin_cookies
     * @return string|null
     */
    public function get_device($plugin_cookies)
    {
        return isset($plugin_cookies->device_number) ? $plugin_cookies->device_number : '1';
    }
}

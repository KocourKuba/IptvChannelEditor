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

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = $this->get_password($plugin_cookies);

        if (empty($password)) {
            hd_print("Password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $password, $this->get_device($plugin_cookies));
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

    /**
     * @param $device
     * @param $plugin_cookies
     */
    public function set_device($device, $plugin_cookies)
    {
        $plugin_cookies->device_number = $device;
    }
}

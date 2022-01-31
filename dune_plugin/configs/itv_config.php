<?php
require_once 'default_config.php';

class ItvPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'https://itv.ooo/p/%s/hls.m3u8';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[BALANCE_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/(?<id>.+)/[^\?]+\?token=(?<token>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{ID}/video.m3u8?token={TOKEN}';
        static::$FEATURES[SQUARE_ICONS] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'res';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'startTime';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'stopTime';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'desc';
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = parent::TransformStreamUrl($plugin_cookies, $archive_ts, $channel);

        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "archive-$archive_ts-10800.m3u8", $url);
                }
                break;
            case 'mpeg':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "archive-$archive_ts-10800.ts", $url);
                }
                else {
                    $url = str_replace('video.m3u8', 'mpegts', $url);
                }
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("Password not set");
        }

        return sprintf(self::PLAYLIST_TV_URL, $password);
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param array &$account_data
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool true if information collected and status valid
     */
    public static function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        // this account has special API to get account info
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if ($force === false && !empty($password)) {
            return true;
        }

        if (empty($password)) {
            hd_print("Password not set");
            return false;
        }

        try {
            $url = sprintf('http://api.itv.live/data/%s', $password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            return false;
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"), true);
        return isset($account_data['package_info']) && !empty($account_data['package_info']);
    }

    public static function AddSubscriptionUI(&$defs, $plugin_cookies)
    {
        $account_data = array();
        $result = self::GetAccountInfo($plugin_cookies, $account_data, true);
        if ($result === false || empty($account_data)) {
            hd_print("Can't get account status");
            $text = 'Невозможно отобразить данные о подписке.\\nНеправильные логин или пароль.';
            $text = explode('\\n', $text);
            $text = array_values($text);

            ControlFactory::add_label($defs, 'Ошибка!', $text[0]);
            ControlFactory::add_label($defs, 'Описание:', $text[1]);
            return;
        }

        $title = 'Пакеты: ';

        ControlFactory::add_label($defs, 'Баланс:', $account_data['user_info']['cash'] . ' $');
        $packages = $account_data['package_info'];
        if (count($packages) === 0) {
            ControlFactory::add_label($defs, $title, 'Нет пакетов');
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

            ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, $collected);

            if ($isFirstLabel) {
                $isFirstLabel = false;
            }

            $list_collected = array();
        }

        if (count($list_collected) !== 0) {
            ControlFactory::add_label($defs, $isFirstLabel ? $title : $emptyTitle, implode(', ', $list_collected));
        }
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            return sprintf('http://api.itv.live/epg/%s', $id); // epg_id)
        }

        return null;
    }
}

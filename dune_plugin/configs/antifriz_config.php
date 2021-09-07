<?php
require_once 'default_config.php';

class AntifrizPluginConfig extends DefaultConfig
{
    // local parameters
    const ACCOUNT_PLAYLIST_URL = 'http://%s/playlist/%s.m3u8';
    const ACCOUNT_PRIMARY_DOMAIN = 'antifriz.tv';
    const STREAM_URL_PATTERN = '/^https?:\/\/(.+)\/s\/(.+)\/.+\/video\.m3u8$/';

    // info
    public static $PLUGIN_NAME = 'AntiFriz TV';
    public static $PLUGIN_SHORT_NAME = 'antifriz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '07.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $USE_LOGIN_PASS = true;

    // tv
    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/s/{TOKEN}/{ID}/video.m3u8';
    public static $MEDIA_URL_TEMPLATE_TS = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $CHANNELS_LIST = 'antifriz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/antifriz/epg/%s.json'; // epg_id date(YYYYMMDD)

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function GetAccessInfo($plugin_cookies)
    {
        hd_print("Collect information from account antifriz");
        $found = false;
        if (!empty($plugin_cookies->login) && !empty($plugin_cookies->password)) {
            try {
                $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                    self::ACCOUNT_PRIMARY_DOMAIN,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                hd_print("Failed to fetch provider playlist");
                return false;
            }

            $tmp_file = static::GET_TMP_STORAGE_PATH('playlist.m3u8');
            file_put_contents($tmp_file, $content);
            $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            for ($i = 0; $i < count($lines); ++$i) {
                hd_print($lines);
                if (preg_match(self::STREAM_URL_PATTERN, $lines[$i], $matches)) {
                    $plugin_cookies->subdomain_local = $matches[1];
                    $plugin_cookies->ott_key_local = $matches[2];
                    hd_print("info: $plugin_cookies->subdomain_local, $plugin_cookies->ott_key_local");
                    $found = true;
                    break;
                }
            }
            unlink($tmp_file);
        }

        return $found;
    }
}

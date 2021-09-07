<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    const ACCOUNT_PLAYLIST_URL = 'http://sharavoz.tk/iptv/p/%s/Sharavoz.Tv.navigator-ott.m3u';
    const STREAM_URL_PATTERN = '/^https?:\/\/(.+)\/.*\/index\.m3u8\?token=(.+)$/';

    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '07.09.2021';

    public static $MPEG_TS_SUPPORTED = true;
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_MPEG = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $CHANNELS_LIST = 'sharavoz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

    public static function GetAccessInfo($plugin_cookies)
    {
        hd_print("Collect information from account");
        $found = false;
        if (!empty($plugin_cookies->password)) {
            try {
                $url = sprintf(self::ACCOUNT_PLAYLIST_URL, $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                hd_print("Failed to fetch provider playlist");
                return false;
            }

            $tmp_file = static::GET_TMP_STORAGE_PATH('playlist.m3u8');
            file_put_contents($tmp_file, $content);
            $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            for ($i = 0; $i < count($lines); ++$i) {
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

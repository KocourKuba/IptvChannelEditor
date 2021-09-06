<?php
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    // local parameters
    const ACCOUNT_PLAYLIST_URL = 'http://%s/get.php?username=%s&password=%s&type=m3u&output=hls';
    const ACCOUNT_PRIMARY_DOMAIN = 'pl.ottglanz.tv';
    const STREAM_URL_PATTERN = '/^https?:\/\/(.+)\/\d+\/(?:mpegts|.+\.m3u8)\?username=.+&password=.+&token=(.+)&ch_id=\d+&req_host=(.+)$/';

    // info
    public static $PLUGIN_NAME = 'Glanz TV';
    public static $PLUGIN_SHORT_NAME = 'glanz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '01.09.2021';

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $USE_LOGIN_PASS = true;

    // tv
    public static $MEDIA_URL_TEMPLATE = 'http://{SUBDOMAIN}/{ID}/index.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    public static $CHANNELS_LIST = 'glanz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id date(YYYYMMDD)

    // Views variables
    public static $TV_CHANNEL_ICON_WIDTH = 60;
    public static $TV_CHANNEL_ICON_HEIGHT = 60;

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "&utc=$archive_ts&lutc=$now_ts";
        }

        $url = str_replace('{LOGIN}', $plugin_cookies->login, $url);
        $url = str_replace('{PASSWORD}', $plugin_cookies->password, $url);
        $url = str_replace('{HOST}', $plugin_cookies->host, $url);

        switch ($format) {
            case 'hls':
                $url = str_replace('index.m3u8', 'video.m3u8', $url);
                break;
            case 'mpeg':
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url = str_replace('index.m3u8', 'mpegts', $url);
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
        }

        return $url;
    }

    public static function GetAccessInfo($plugin_cookies)
    {
        hd_print("Collect information from account glanz");
        $found = false;
        if (!empty($plugin_cookies->login) && !empty($plugin_cookies->password)) {
            try {
                $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                    self::ACCOUNT_PRIMARY_DOMAIN,
                    $plugin_cookies->login,
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
                    $plugin_cookies->host = $matches[3];
                    hd_print("info: $plugin_cookies->subdomain_local, $plugin_cookies->ott_key_local, $plugin_cookies->host");
                    $found = true;
                    break;
                }
            }
            unlink($tmp_file);
        }

        return $found;
    }
}

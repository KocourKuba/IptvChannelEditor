<?php
require_once 'lib/default_config.php';

class SharaclubPluginConfig extends DefaultConfig
{
    const PLUGIN_NAME = 'Sharaclub TV';
    const PLUGIN_SHORT_NAME = 'sharaclub';
    const PLUGIN_VERSION = '1.0.0';
    const PLUGIN_DATE = '01.09.2021';

    const MPEG_TS_SUPPORTED = true;
    const USE_TOKEN = true;

    const MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/live/{TOKEN}/{ID}/video.m3u8';
    const CHANNEL_LIST_URL = 'sharaclub_channel_list.xml';
    const EPG1_URL_FORMAT = 'http://api.sramtv.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)
    const EPG2_URL_FORMAT = 'http://api.gazoni1.com/get/?type=epg&ch=%s&date=%s'; // epg_id date(YYYYMMDD)

    const ACCOUNT_INFO_URL = 'http://%s/api/dune-api5m.php?subscr=%s-%s';
    const ACCOUNT_PLAYLIST_URL = 'http://%s/tv_live-m3u8/%s-%s';
    const ACCOUNT_PRIMARY_DOMAIN = 'list.playtv.pro';
    const ACCOUNT_SECONDARY_DOMAIN = 'list.shara.tv';

    const STREAM_URL_PATTERN = '/^https?:\/\/(.+)\/live\/(.+)\/.+\/.*\.m3u8$/';

    public final function AdjustStreamUri($plugin_cookies, $archive_ts, $url)
    {
        $format = isset($plugin_cookies->format) ? $plugin_cookies->format : 'hls';
        hd_print("AdjustStreamUri: using stream format to '$format'");

        if (intval($archive_ts) > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
        }

        switch ($format)
        {
            case 'hls':
                break;
            case 'mpeg':
                $url = str_replace('/video.m3u8', '.ts', $url);
                break;
        }

        return $url;
    }

    public function GetAccountStatus($plugin_cookies)
    {
        if (empty($plugin_cookies->login) && empty($plugin_cookies->password))
            return false;

        try {
            $url = sprintf(self::ACCOUNT_INFO_URL,
                self::ACCOUNT_PRIMARY_DOMAIN,
                $plugin_cookies->login,
                $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            try {
                $url = sprintf(self::ACCOUNT_INFO_URL,
                    self::ACCOUNT_SECONDARY_DOMAIN,
                    $plugin_cookies->login,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                return false;
            }
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"));
        if (isset($account_data->status) && $account_data->status == 'ok') {
            hd_print("account ok");
            return $account_data;
        }

        return false;
    }

    public static function GetAccessInfo($plugin_cookies)
    {
        hd_print("Collect information from account");
        $found = false;
        if (!empty($plugin_cookies->login) && !empty($plugin_cookies->password)) {
            try {
                $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                    self::ACCOUNT_PRIMARY_DOMAIN,
                    $plugin_cookies->login,
                    $plugin_cookies->password);
                $content = HD::http_get_document($url);
            } catch (Exception $ex) {
                try {
                    $url = sprintf(self::ACCOUNT_PLAYLIST_URL,
                        self::ACCOUNT_SECONDARY_DOMAIN,
                        $plugin_cookies->login,
                        $plugin_cookies->password);
                    $content = HD::http_get_document($url);
                } catch (Exception $ex) {
                    return false;
                }
            }

            $tmp_file = '/tmp/playlist.tmp';
            file_put_contents($tmp_file, $content);
            $lines = file($tmp_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            for ($i = 0; $i < count($lines); ++$i) {
                if (preg_match(self::STREAM_URL_PATTERN, $lines[$i], $matches)) {
                    $plugin_cookies->subdomain_local = $matches[1];
                    $plugin_cookies->ott_key_local = $matches[2];
                    $found = true;
                    break;
                }
            }
            unlink($tmp_file);
        }

        return $found;
    }
}

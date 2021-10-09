<?php
require_once 'default_config.php';

class ItvPluginConfig extends DefaultConfig
{
    const ACCOUNT_INFO_URL1 = 'http://api.itv.live/data/%s';
    const EXTINF_TV_PATTERN  = '|^#EXTINF:.+CUID="(\d+)".+$|';

    // info
    public static $PLUGIN_NAME = 'ITV Live TV';
    public static $PLUGIN_SHORT_NAME = 'itv';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '12.10.2021';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = false;
    public static $VOD_FAVORITES_SUPPORTED = false;

    // setup variables
    public static $MPEG_TS_SUPPORTED = true;
    public static $ACCOUNT_TYPE = 'PIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'https://itv.ooo/p/%s/hls.m3u8';
    public static $STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>.+)/[^\?]+\?token=(?<token>.+)$|';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/{ID}}/video.m3u8?token={TOKEN}';
    public static $CHANNELS_LIST = 'itv_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.itv.live/epg/%s';

    public function __construct()
    {
        parent::__construct();
        static::$EPG_PARSER_PARAMS['epg_root'] = 'res';
        static::$EPG_PARSER_PARAMS['start'] = 'startTime';
        static::$EPG_PARSER_PARAMS['end'] = 'stopTime';
        static::$EPG_PARSER_PARAMS['title'] = 'title';
        static::$EPG_PARSER_PARAMS['description'] = 'desc';
    }

    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        // this account has special API to get account info
        if (empty($plugin_cookies->password))
            return false;

        try {
            $url = sprintf(static::ACCOUNT_INFO_URL1, $plugin_cookies->password);
            $content = HD::http_get_document($url);
        } catch (Exception $ex) {
            return false;
        }

        $account_data = json_decode(ltrim($content, "\0xEF\0xBB\0xBF"));
        return isset($account_data->package_info) && !empty($account_data->package_info);
    }

    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $format = static::get_format($plugin_cookies);
        $url = $channel->get_streaming_url();
        //hd_print("AdjustStreamUrl: $url");
        switch ($format) {
            case 'hls':
                break;
            case 'mpeg':
                // replace hls to mpegts
                $url = str_replace('/video.m3u8', 'mpegts', $url);
                $buf_time = isset($plugin_cookies->buf_time) ? $plugin_cookies->buf_time : '1000';
                $url .= "|||dune_params|||buffering_ms:$buf_time";
                break;
            default:
                hd_print("unknown format: $format");
                return "";
        }

        if (intval($archive_ts) > 0) {
            $url .= '&utc={START}&lutc={NOW}';
        }

        // hd_print("Stream type: " . $format);
        // hd_print("Stream url:  " . $url);
        // hd_print("Channel ID:  " . $id);
        // hd_print("Archive TS:  " . $archive_ts);

        $url = str_replace('{START}', $archive_ts, $url);
        $url = str_replace('{START}', $archive_ts, $url);
        $url = str_replace('{NOW}', strval(time()), $url);

        return static::make_ts($url);
    }

    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $m3u_lines = static::FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $line) {
            if (preg_match(self::$STREAM_URL_PATTERN, $line, $matches)) {
                $pl_entries[$matches['id']] = $line;
            }
        }

        hd_print("Read Playlist entries: " . count($pl_entries));
        return $pl_entries;
    }
}

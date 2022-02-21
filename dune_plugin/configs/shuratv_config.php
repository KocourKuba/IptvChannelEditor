<?php
require_once 'default_config.php';

class ShuratvPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://pl.tvshka.net/?uid=%s&srv=%d&type=halva';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'PIN';
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8';
        static::$FEATURES[SERVER_SUPPORTED] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = '';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'start_time';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'duration';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'name';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'text';
        static::$EPG_PARSER_PARAMS['first']['use_duration'] = true;
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
        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

        if (self::get_format($plugin_cookies) === 'mpeg') {
            $url = str_replace('/hls/pl.m3u8', '', $url);
        }

        // hd_print("Stream url:  " . $url);

        return self::UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;
        if (empty($password)) {
            hd_print("User password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_TV_URL, $password, static::get_server($plugin_cookies));
    }

    protected static function UpdateArchiveUrlParams($url, $archive_ts)
    {
        if ($archive_ts > 0) {
            $now_ts = time();
            $url .= (strpos($url, '?') === false) ? '?' : '&';
            $url .= "archive=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    public static function get_server_opts($plugin_cookies)
    {
        return array('1', '2');
    }

    public static function get_server($plugin_cookies)
    {
        return isset($plugin_cookies->server) ? $plugin_cookies->server : 0;
    }

    public static function set_server($plugin_cookies)
    {
        return true;
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if($type === 'first') {
            hd_print("Fetching EPG for ID: '$id'");
            return sprintf('http://s1.tvshka.net/%s/epg/range14-7.json', $id);
        }

        return null;
    }
}

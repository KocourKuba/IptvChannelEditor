<?php
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    const PLAYLIST_VOD_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=vod';
    const API_URL = 'http://technic.cf/epg-iptvxone';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = true;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
        static::$FEATURES[EXTINF_VOD_PATTERN] = '|^#EXTINF.+group-title="(?<category>.*)".+tvg-logo="(?<logo>.*)"\s*,\s*(?<title>.*)$|';
        static::$FEATURES[SQUARE_ICONS] = true;

        static::$EPG_PARSER_PARAMS['first']['epg_root'] = 'data';
        static::$EPG_PARSER_PARAMS['first']['start'] = 'begin';
        static::$EPG_PARSER_PARAMS['first']['end'] = 'end';
        static::$EPG_PARSER_PARAMS['first']['title'] = 'title';
        static::$EPG_PARSER_PARAMS['first']['description'] = 'description';
        static::$EPG_PARSER_PARAMS['first']['date_format'] = 'Y.m.d';
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
        $ext_params = $channel->get_ext_params();
        $url = str_replace(
            array(
                '{LOGIN}',
                '{PASSWORD}',
                '{INT_ID}',
                '{HOST}'
            ),
            array(
                $ext_params['login'],
                $ext_params['password'],
                $ext_params['int_id'],
                $ext_params['host']
            ),
            $url);

        switch (self::get_format($plugin_cookies)) {
            case 'hls':
                if ((int)$archive_ts > 0) {
                    // hd_print("Archive TS:  " . $archive_ts);
                    $url = str_replace('video.m3u8', "video-$archive_ts-10800.m3u8", $url);
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

    public function GetAccountInfo(&$plugin_cookies, &$account_data, $force = false)
    {
        if (!parent::GetAccountInfo($plugin_cookies, &$account_data, $force)) {
            return false;
        }

        try {
            $content = HD::http_get_document(self::API_URL . '/channels');
            // stripe UTF8 BOM if exists
            $json_data = json_decode(ltrim($content, "\xEF\xBB\xBF"), true);
            static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper'] = $json_data['data'];
            hd_print("TVG ID Mapped: " . count(static::$EPG_PARSER_PARAMS['first']['tvg_id_mapper']));
        } catch (Exception $ex) {
            return false;
        }

        return true;
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::PLAYLIST_TV_URL, $login, $password);
            case 'movie':
                return sprintf(self::PLAYLIST_VOD_URL, $login, $password);
        }

        return '';
    }

    public static function get_epg_url($type, $id, $day_start_ts, $plugin_cookies)
    {
        if ($type === 'first') {
            $epg_date = gmdate(static::$EPG_PARSER_PARAMS['first']['date_format'], $day_start_ts);
            hd_print("Fetching EPG for ID: '$id' DATE: $epg_date");
            return sprintf('%s/epg_day?id=%s&day=%s', self::API_URL, $id, $epg_date); // epg_id date(Y.m.d)
        }

        return null;
    }

    /**
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        //hd_print("Movie ID: $movie_id");
        $movie = new Movie($movie_id);
        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if ($i !== (int)$movie_id || !preg_match(static::$FEATURES[EXTINF_VOD_PATTERN], $line, $matches)) {
                continue;
            }

            $logo = $matches['logo'];
            $caption = $matches['title'];

            $url = $m3u_lines[$i + 1];
            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $caption,// $xml->caption,
                '',// $xml->caption_original,
                '',// $xml->description,
                $logo,// $xml->poster_url,
                '',// $xml->length,
                '',// $xml->year,
                '',// $xml->director,
                '',// $xml->scenario,
                '',// $xml->actors,
                '',// $xml->genres,
                '',// $xml->rate_imdb,
                '',// $xml->rate_kinopoisk,
                '',// $xml->rate_mpaa,
                '',// $xml->country,
                ''// $xml->budget
            );

            $movie->add_series_data($movie_id, $caption, $url, true);
            hd_print("movie url: $url");
            break;
        }

        return $movie;
    }
}

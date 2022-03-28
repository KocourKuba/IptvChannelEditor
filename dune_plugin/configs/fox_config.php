<?php
require_once 'default_config.php';

class FoxPluginConfig extends DefaultConfig
{
    const PLAYLIST_TV_URL = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';
    const PLAYLIST_VOD_URL = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';
    const API_URL = 'http://technic.cf/epg-fox';

    public function __construct()
    {
        parent::__construct();

        static::$FEATURES[ACCOUNT_TYPE] = 'LOGIN';
        static::$FEATURES[TS_OPTIONS] = array('hls' => 'HLS');
        static::$FEATURES[VOD_MOVIE_PAGE_SUPPORTED] = true;
        static::$FEATURES[VOD_FAVORITES_SUPPORTED] = true;
        static::$FEATURES[M3U_STREAM_URL_PATTERN] = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/?(?<hls>.+\.m3u8){0,1}$|';
        static::$FEATURES[MEDIA_URL_TEMPLATE_HLS] = 'http://{DOMAIN}/{ID}/{TOKEN}/index.m3u8';
        static::$FEATURES[EXTINF_VOD_PATTERN] = '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|';
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
        $url = $channel->get_streaming_url();
        $ext_params = $channel->get_ext_params();

        if (!isset($ext_params[0])) {
            hd_print("TransformStreamUrl: parameters for {$channel->get_channel_id()} not defined!");
        } else {
            // fox does not have adjustable parameters, only token. Replace entire url from playlist
            $url = $ext_params[0];
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

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

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information for: $this->PLUGIN_SHOW_NAME");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if (preg_match('|^#EXTINF:.+CUID="(?<id>\d+)"|', $iValue, $m_id)
                && preg_match(static::$FEATURES[M3U_STREAM_URL_PATTERN], $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            if (file_exists($this->GET_TMP_STORAGE_PATH())) {
                unlink($this->GET_TMP_STORAGE_PATH());
            }
        }

        return $pl_entries;
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
        // hd_print("Movie ID: $movie_id ");
        $movie = new Movie($movie_id);

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if ($i !== (int)$movie_id || !preg_match(static::$FEATURES[EXTINF_VOD_PATTERN], $iValue, $match)) {
                continue;
            }

            $logo = $match['logo'];
            list($title, $title_orig) = explode('/', $match['title']);
            $url = self::UpdateMpegTsBuffering($m3u_lines[$i + 1], $plugin_cookies);

            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $title,// $xml->caption,
                $title_orig,// $xml->caption_original,
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

            $movie->add_series_data($movie_id, $title, $url, true);
            // hd_print("movie_id: $movie_id");
            // hd_print("title: $title");
            hd_print("movie url: $url");
            break;
        }

        return $movie;
    }
}

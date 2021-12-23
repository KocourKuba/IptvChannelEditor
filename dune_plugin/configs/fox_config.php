<?php
require_once 'default_config.php';

class FoxPluginConfig extends DefaultConfig
{
    // supported features
    public static $ACCOUNT_TYPE = 'LOGIN';
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // tv
    protected static $PLAYLIST_TV_URL = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';
    protected static $PLAYLIST_VOD_URL = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/?(?<hls>.+\.m3u8){0,1}$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{DOMAIN}/{ID}/{TOKEN}/index.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/fox-tv/epg/%s.json'; // epg_id

    // vod
    public static $EXTINF_VOD_PATTERN = '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|';

    // Views variables
    protected static $TV_CHANNEL_ICON_WIDTH = 60;
    protected static $TV_CHANNEL_ICON_HEIGHT = 60;

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

        $url = HD::make_ts(self::UpdateArchiveUrlParams($url, $archive_ts));

        if (!isset($ext_params['hls'])) {
            $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    protected static function GetPlaylistUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        switch ($type) {
            case 'tv1':
                return sprintf(self::$PLAYLIST_TV_URL, $login, $password);
            case 'movie':
                return sprintf(self::$PLAYLIST_VOD_URL, $login, $password);
        }

        return '';
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        $pl_entries = array();
        $m3u_lines = self::FetchTvM3U($plugin_cookies);
        for ($i = 0, $iMax = count($m3u_lines); $i < $iMax; ++$i) {
            if (preg_match('|^#EXTINF:.+CUID="(?<id>\d+)"|', $m3u_lines[$i], $m_id)
                && preg_match(self::$M3U_STREAM_URL_PATTERN, $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            throw new DuneException(
                'Empty provider playlist', 0,
                ActionFactory::show_error(
                    true,
                    'Ошибка скачивания плейлиста',
                    array(
                        'Пустой плейлист провайдера!',
                        'Проверьте подписку или подключение к Интернет.')));
        }

        return $pl_entries;
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        // hd_print("Movie ID: $movie_id ");
        $movie = new Movie($movie_id);

        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        for ($i = 0, $iMax = count($m3u_lines); $i < $iMax; ++$i) {
            if ($i !== (int)$movie_id || !preg_match(static::$EXTINF_VOD_PATTERN, $m3u_lines[$i], $match)) {
                continue;
            }

            $logo = $match['logo'];
            list($title, $title_orig) = explode('/', $match['title']);
            $url = HD::make_ts(self::UpdateMpegTsBuffering($m3u_lines[$i + 1], $plugin_cookies));

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

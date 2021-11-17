<?php
/** @noinspection ForeachInvariantsInspection */
require_once 'default_config.php';

class FoxPluginConfig extends DefaultConfig
{
    const SERIES_VOD_PATTERN = '|^https?://.+/vod/(.+)\.mp4/video\.m3u8\?token=.+$|';
    const EXTINF_VOD_PATTERN = '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|';
    const EXTINF_TV_PATTERN  = '|^#EXTINF:.+CUID="(?<id>\d+)".+$|';

    // supported features
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // setup variables
    public static $ACCOUNT_TYPE = 'LOGIN';

    // account
    public static $ACCOUNT_PLAYLIST_URL1 = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/?(?<hls>.+\.m3u8){0,1}$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/{TOKEN}/index.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/fox-tv/epg/%s.json'; // epg_id

    // vod
    protected static $MOVIE_LIST_URL_TEMPLATE = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';

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
            if (preg_match(self::EXTINF_TV_PATTERN, $m3u_lines[$i], $m_id)
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
            if ($i !== (int)$movie_id || !preg_match(self::EXTINF_VOD_PATTERN, $m3u_lines[$i], $match)) {
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

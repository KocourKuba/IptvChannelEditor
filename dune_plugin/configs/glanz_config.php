<?php
require_once 'default_config.php';

class GlanzPluginConfig extends DefaultConfig
{
    // supported features
    public static $ACCOUNT_TYPE = 'LOGIN';
    public static $MPEG_TS_SUPPORTED = true;
    public static $VOD_MOVIE_PAGE_SUPPORTED = true;
    public static $VOD_FAVORITES_SUPPORTED = true;

    // tv
    public static $M3U_STREAM_URL_PATTERN = '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|';
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/ottg/epg/%s.json'; // epg_id

    // vod
    public static $EXTINF_VOD_PATTERN = '|^#EXTINF.+group-title="(?<category>.*)".+tvg-logo="(?<logo>.*)"\s*,\s*(?<title>.*)$|';

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
                $url = self::UpdateMpegTsBuffering($url, $plugin_cookies);
                break;
            default:
                hd_print("unknown url format");
                return "";
        }

        // hd_print("Stream url:  " . $url);

        return $url;
    }

    protected static function GetTemplatedUrl($type, $plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = empty($plugin_cookies->login_local) ? $plugin_cookies->login : $plugin_cookies->login_local;
        $password = empty($plugin_cookies->password_local) ? $plugin_cookies->password : $plugin_cookies->password_local;

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        switch ($type) {
            case 'tv1':
                return sprintf('http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls', $login, $password);
            case 'movie':
                return sprintf('http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=vod', $login, $password);
        }

        return '';
    }

    /**
     * @throws Exception
     */
    public static function TryLoadMovie($movie_id, $plugin_cookies)
    {
        //hd_print("Movie ID: $movie_id");
        $movie = new Movie($movie_id);
        $m3u_lines = static::FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if ($i !== (int)$movie_id || !preg_match(static::$EXTINF_VOD_PATTERN, $line, $matches)) {
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

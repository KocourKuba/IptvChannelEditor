<?php
require_once 'default_config.php';

class GlanzPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    const PLAYLIST_VOD_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=vod';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(EXTINF_VOD_PATTERN, '|^#EXTINF.+group-title="(?<category>.*)".+tvg-logo="(?<logo>.*)"\s*,\s*(?<title>.*)$|');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('first','epg_url', 'http://epg.iptvx.one/api/id/{CHANNEL}.json');
        $this->set_epg_param('first','epg_root', 'ch_programme');
        $this->set_epg_param('first','epg_start', 'start');
        $this->set_epg_param('first','epg_end', '');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_time_format', 'd-m-Y H:i');
        $this->set_epg_param('first','epg_timezone', 'Europe/Moscow');

        $this->set_epg_param('second','epg_url', 'http://technic.cf/epg-iptvxone/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('second','epg_root', 'data');
        $this->set_epg_param('second','epg_start', 'begin');
        $this->set_epg_param('second','epg_end', 'end');
        $this->set_epg_param('second','epg_title', 'title');
        $this->set_epg_param('second','epg_description', 'description');
        $this->set_epg_param('second','epg_date_format', 'Y.m.d');
        $this->set_epg_param('second','epg_use_mapper', true);
        $this->set_epg_param('second','epg_mapper_url', 'http://technic.cf/epg-iptvxone/channels');
    }

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function TransformStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
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

        switch ($this->get_format($plugin_cookies)) {
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

        return $this->UpdateMpegTsBuffering($url, $plugin_cookies);
    }

    /**
     * @param string $type
     * @param $plugin_cookies
     * @return string
     */
    protected function GetPlaylistUrl($type, $plugin_cookies)
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
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        hd_print("TryLoadMovie: $movie_id");
        $movie = new Movie($movie_id);
        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $line) {
            if ($i !== (int)$movie_id || !preg_match($this->get_feature(EXTINF_VOD_PATTERN), $line, $matches)) {
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

            $movie->add_series_data($movie_id, $caption, $url);
            hd_print("movie url: $url");
            break;
        }

        return $movie;
    }
}

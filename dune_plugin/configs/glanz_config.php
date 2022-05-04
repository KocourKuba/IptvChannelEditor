<?php
require_once 'default_config.php';

class GlanzPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=hls';
    const PLAYLIST_VOD_URL = 'http://pl.ottglanz.tv/get.php?username=%s&password=%s&type=m3u&output=vod';

    protected $server_opts;

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(SERVER_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>.+)/(?<id>\d+)/.+\.m3u8\?username=(?<login>.+)&password=(?<password>.+)&token=(?<token>.+)&ch_id=(?<int_id>\d+)&req_host=(?<host>.+)$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/video.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_HLS, 'http://{DOMAIN}/{ID}/video-{START}-10800.m3u8?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_MPEG, 'http://{DOMAIN}/{ID}/mpegts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
        $this->set_feature(MEDIA_URL_TEMPLATE_ARCHIVE_MPEG, 'http://{DOMAIN}/{ID}/archive-{START}-10800.ts?username={LOGIN}&password={PASSWORD}&token={TOKEN}&ch_id={INT_ID}&req_host={HOST}');
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

        $this->server_opts = array(
            array(
                "Austria",
                "Czechia",
                "England",
                "Germany 0",
                "Germany 1",
                "Germany 2",
                "Germany 3",
                "Netherlands",
                "Italy",
                "Poland 1",
                "Poland 2",
                "Romania",
                "Russia 1",
                "Russia 2",
                "Russia, Moscow",
                "Russia, Khabarovsk",
                "Sweden",
                "Ukraine 1",
                "Ukraine 2",
                "USA"
            ),
            array(
                "str16.ottg.tv",	// Austria
                "str07.ottg.tv",	// Czechia
                "str13.ottg.tv",	// England
                "cdn.ottg.tv",		// Germany 0
                "str01.ottg.tv",	// Germany 1
                "str02.ottg.tv",	// Germany 2
                "str11.ottg.tv",	// Germany 3
                "str08.ottg.tv",	// Netherlands
                "str19.ottg.tv",	// Italy
                "str06.ottg.tv",	// Poland 1
                "str14.ottg.tv",	// Poland 2
                "str10.ottg.tv",	// Romania
                "str05.ottg.tv",	// Russia 1
                "str09.ottg.tv",	// Russia 2
                "str17.ottg.tv",	// Russia, Moscow
                "str18.ottg.tv",	// Russia, Khabarovsk
                "str03.ottg.tv",	// Sweden
                "str04.ottg.tv",	// Ukraine 1
                "str15.ottg.tv",	// Ukraine 2
                "str12.ottg.tv" 	// USA
            )
        );
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
        $url = $channel->get_streaming_url();
        if (!empty($url)) {
            $url = ((int)$archive_ts <= 0) ?: static::UpdateArchiveUrlParams($url, $archive_ts);
        } else {
            switch ($this->get_format($plugin_cookies)) {
                case 'hls':
                    $url = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_HLS : MEDIA_URL_TEMPLATE_HLS);
                    break;
                case 'mpeg':
                    $url = $this->get_feature(((int)$archive_ts > 0) ? MEDIA_URL_TEMPLATE_ARCHIVE_MPEG : MEDIA_URL_TEMPLATE_MPEG);
                    break;
                default:
                    hd_print("unknown url format");
                    return "";
            }

            $ext_params = $channel->get_ext_params();
            $url = str_replace(
                array(
                    '{DOMAIN}',
                    '{ID}',
                    '{LOGIN}',
                    '{PASSWORD}',
                    '{TOKEN}',
                    '{INT_ID}',
                    '{HOST}',
                    '{START}'
                ),
                array(
                    $this->get_subst_server($plugin_cookies),
                    $channel->get_channel_id(),
                    $ext_params['login'],
                    $ext_params['password'],
                    $ext_params['token'],
                    $ext_params['int_id'],
                    $ext_params['host'],
                    $archive_ts
                ),
                $url);
        }

        // hd_print("Stream url:  $url");

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

        $login = isset($this->embedded_account->login) ? $this->embedded_account->login : $plugin_cookies->login;
        $password = isset($this->embedded_account->password) ? $this->embedded_account->password : $plugin_cookies->password;

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

            $movie->add_series_data($movie_id, $caption, '', $url);
            hd_print("movie url: $url");
            break;
        }

        return $movie;
    }

    /**
     * @param $plugin_cookies
     * @return string[]
     */
    public function get_server_opts($plugin_cookies)
    {
        return $this->server_opts[0];
    }

    /**
     * @param $plugin_cookies
     * @return int|null
     */
    public function get_server($plugin_cookies)
    {
        return isset($plugin_cookies->server) ? $plugin_cookies->server : 0;
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function get_subst_server($plugin_cookies)
    {
        return $this->server_opts[1][$this->get_server($plugin_cookies)];
    }
}

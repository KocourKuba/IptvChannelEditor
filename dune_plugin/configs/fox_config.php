<?php
require_once 'default_config.php';

class FoxPluginConfig extends Default_Config
{
    const PLAYLIST_TV_URL = 'http://pl.fox-tv.fun/%s/%s/tv.m3u';
    const PLAYLIST_VOD_URL = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';

    public function __construct()
    {
        parent::__construct();

        $this->set_feature(ACCOUNT_TYPE, 'LOGIN');
        $this->set_feature(TS_OPTIONS, array('hls' => 'HLS'));
        $this->set_feature(VOD_MOVIE_PAGE_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(M3U_STREAM_URL_PATTERN, '|^https?://(?<subdomain>[^/]+)/(?<token>[^/]+)/?(?<hls>.+\.m3u8){0,1}$|');
        $this->set_feature(MEDIA_URL_TEMPLATE_HLS, 'http://{DOMAIN}/{ID}/{TOKEN}/index.m3u8');
        $this->set_feature(EXTINF_VOD_PATTERN, '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|');
        $this->set_feature(SQUARE_ICONS, true);

        $this->set_epg_param('first','epg_url','http://technic.cf/epg-fox/epg_day?id={CHANNEL}&day={DATE}');
        $this->set_epg_param('first','epg_root', 'data');
        $this->set_epg_param('first','epg_start', 'begin');
        $this->set_epg_param('first','epg_end', 'end');
        $this->set_epg_param('first','epg_title', 'title');
        $this->set_epg_param('first','epg_desc', 'description');
        $this->set_epg_param('first','epg_date_format', 'Y.m.d');
        $this->set_epg_param('first','epg_use_mapper', true);
        $this->set_epg_param('first','epg_mapper_url','http://technic.cf/epg-fox/channels');
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
        $ext_params = $channel->get_ext_params();

        if (!isset($ext_params[0])) {
            hd_print("TransformStreamUrl: parameters for {$channel->get_channel_id()} not defined!");
        } else {
            // fox does not have adjustable parameters, only token. Replace entire url from playlist
            $url = $ext_params[0];
        }

        $url = static::UpdateArchiveUrlParams($url, $archive_ts);

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
                && preg_match($this->get_feature(M3U_STREAM_URL_PATTERN), $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id['id']] = $matches;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $tmp_file = DuneSystem::$properties['tmp_dir_path'] . "/playlist_tv.m3u8";
            if (file_exists($tmp_file)) {
                unlink($tmp_file);
            }
        }

        return $pl_entries;
    }

    /**
     * @param string $movie_id
     * @param $plugin_cookies
     * @return Movie
     * @throws Exception
     */
    public function TryLoadMovie($movie_id, $plugin_cookies)
    {
        // hd_print("Movie ID: $movie_id ");
        $movie = new Movie($movie_id);

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if ($i !== (int)$movie_id || !preg_match($this->get_feature(EXTINF_VOD_PATTERN), $iValue, $match)) {
                continue;
            }

            $logo = $match['logo'];
            list($title, $title_orig) = explode('/', $match['title']);
            $url = $this->UpdateMpegTsBuffering($m3u_lines[$i + 1], $plugin_cookies);

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

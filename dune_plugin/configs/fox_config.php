<?php
require_once 'lib/default_config.php';

class fox_config extends default_config
{
    const PLAYLIST_VOD_URL = 'http://pl.fox-tv.fun/%s/%s/vodall.m3u';

    public function load_default()
    {
        $this->set_feature(ACCOUNT_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(VOD_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(VOD_PATTERN, '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|');
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/tv.m3u');
        $this->set_feature(URI_PARSE_TEMPLATE, '|^https?://(?<domain>[^/]+)/(?<token>.+)$|');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.drm-play.ml/fox-tv/epg/{ID}.json');
    }

    /**
     * @param $plugin_cookies
     * @return string
     */
    protected function GetVodListUrl($plugin_cookies)
    {
        // hd_print("Type: $type");

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
            return '';
        }

        return sprintf(self::PLAYLIST_VOD_URL, $login, $password);
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     * @throws Exception
     */
    public function GetPlaylistStreamInfo($plugin_cookies)
    {
        hd_print("Get playlist information");
        $pl_entries = array();
        $m3u_lines = $this->FetchTvM3U($plugin_cookies);
        $skip_next = false;
        foreach ($m3u_lines as $i => $iValue) {
            if ($skip_next) {
                $skip_next = false;
                continue;
            }

            if (preg_match('|^#EXTINF:.+CUID="(?<id>\d+)"|', $iValue, $m_id)
                && preg_match($this->get_feature(URI_PARSE_TEMPLATE), $m3u_lines[$i + 1], $matches)) {
                $pl_entries[$m_id[M_ID]] = $matches;
                $skip_next = true;
            }
        }

        if (empty($pl_entries)) {
            hd_print('Empty provider playlist! No channels mapped.');
            $this->ClearPlaylistCache();
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
        hd_print("TryLoadMovie: $movie_id");
        $movie = new Movie($movie_id);

        $m3u_lines = $this->FetchVodM3U($plugin_cookies);
        foreach ($m3u_lines as $i => $iValue) {
            if ($i !== (int)$movie_id || !preg_match($this->get_feature(VOD_PATTERN), $iValue, $match)) {
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

            $movie->add_series_data($movie_id, $title, '', $url);
            // hd_print("movie_id: $movie_id");
            // hd_print("title: $title");
            hd_print("movie url: $url");
            break;
        }

        return $movie;
    }
}

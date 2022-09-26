<?php
require_once 'lib/default_config.php';

class smile_config extends default_config
{
    public function init_defaults($short_name)
    {
        parent::init_defaults($short_name);

        $this->set_feature(VOD_SUPPORTED, true);
        $this->set_feature(VOD_FAVORITES_SUPPORTED, true);
        $this->set_feature(VOD_PLAYLIST_URL, 'http://pl.smile-tv.live/%s/%s/vodall.m3u');
        $this->set_feature(VOD_PARSE_PATTERN, '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>.*)$|');
    }

    public function load_default()
    {
        $this->set_feature(SQUARE_ICONS, true);
        $this->set_feature(ACCESS_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(USE_TOKEN_AS_ID, true);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://pl.smile-tv.live/{LOGIN}/{PASSWORD}/tv.m3u');
        $this->set_feature(URI_ID_PARSE_PATTERN, '^#EXTINF:.+CUID="(?<id>\d+)"');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>[^/]+)/(?<token>.+)$');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/{TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.esalecrm.net/smile/epg/{ID}.json');
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

        return sprintf($this->get_feature(VOD_PLAYLIST_URL), $login, $password);
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
            if ($i !== (int)$movie_id || !preg_match($this->get_feature(VOD_PARSE_PATTERN), $iValue, $match)) {
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

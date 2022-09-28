<?php
require_once 'lib/default_config.php';

class fox_config extends default_config
{
    public function init_defaults()
    {
        parent::init_defaults();

        $this->set_feature(VOD_SUPPORTED, true);
        $this->set_feature(VOD_PARSE_PATTERN, '|^#EXTINF:.+tvg-logo="(?<logo>[^"]+)".+group-title="(?<category>[^"]+)".*,\s*(?<title>[^\/]+)\/(?<title_orig>.+)\s(?<year>\d+)$|');
        $this->set_feature(VOD_PLAYLIST_URL, 'http://pl.fox-tv.fun/{LOGIN}/{PASSWORD}/vodall.m3u');
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
            $title = $match['title'];
            $title_orig = $match['title_orig'];
            $year = $match['year'];
            $url = $this->UpdateMpegTsBuffering($m3u_lines[$i + 1], $plugin_cookies);

            //hd_print("Vod url: $playback_url");
            $movie->set_data(
                $title,// $xml->caption,
                $title_orig,// $xml->caption_original,
                '',// $xml->description,
                $logo,// $xml->poster_url,
                '',// $xml->length,
                $year,// $xml->year,
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

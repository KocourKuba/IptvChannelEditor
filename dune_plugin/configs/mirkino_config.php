<?php
require_once 'lib/default_config.php';
require_once 'lib/jellyfin/jellyfin_api.php';

/**
 * Mir Kino VOD. JellyFin API
 * API support pagination, search
 * allow to select quality, audio track is selected in player
 * TV Shows can contain seasons
 */
class mirkino_config extends default_config
{
    const PAGE_LIMIT = 50;

    /**
     * Audio codecs played by Dune in MPEG-TS segments. Without AudioCodec the server infers it from the url
     * extension (mp3 for .ts segments) and transcodes every other audio track to mp3.
     * Audio in these codecs is copied, other codecs are transcoded to the first one (aac).
     */
    const HLS_AUDIO_CODECS = 'aac,ac3,eac3,mp3,mp2,dts';

    /**
     * @var jellyfin_api
     */
    protected $jfc;

    /**
     * hash of the server url and credentials used by current session
     * @var string
     */
    protected $session_hash;

    /**
     * VOD only provider, there is no tv playlist
     *
     * @inheritDoc
     */
    public function GetPlaylistStreamsInfo()
    {
        return array();
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print("Collect information from account: " . var_export($force, true));

        if ($force) {
            unset($this->jfc);
        }

        $jfc = $this->get_jfc();
        if ($jfc === null) {
            return false;
        }

        $this->account_data = $jfc->getCurrentUser();
        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function GetVodStreamUrl($playback_url)
    {
        hd_debug_print(null, true);
        hd_debug_print("Playback url: $playback_url", true);

        $jfc = $this->get_jfc();
        if ($jfc === null) {
            return '';
        }

        $media_url = MediaURL::decode($playback_url);

        $query = array();
        if (isset($media_url->stream_id)) {
            $query['MediaSourceId'] = $media_url->stream_id;
        }

        // PlaybackInfo registers the play session, the stream is served only within it
        $info = $jfc->getItemPlaybackInfo($media_url->id, $query);
        if ($info === false) {
            hd_debug_print("Can't get response on Playback info");
        }

        if (isset($info['PlaySessionId'])) {
            $query['PlaySessionId'] = $info['PlaySessionId'];
        }

        // for requested media source the response contains only this source, otherwise the one selected by server
        $source = safe_get_value($info, array('MediaSources', 0), array());
        if (!isset($query['MediaSourceId']) && !empty($source['Id'])) {
            $query['MediaSourceId'] = $source['Id'];
        }

        if (safe_get_value($source, 'SupportsDirectStream', false)) {
            // original file as is: no load on server, all audio tracks are selectable in player.
            // HLS is not used when possible, it fails for alternate versions (other qualities) of the movie
            $url = $jfc->getStreamUrl($media_url->id, $query, self::get_stream_extension(safe_get_value($source, 'Container', '')));
        } else {
            $query['AudioCodec'] = self::HLS_AUDIO_CODECS;
            $url = $jfc->getPlayUrlMaster($media_url->id, $query);
        }
        hd_debug_print("Stream url: $url", true);

        $url = $this->UpdateVodUrlParams(HD::make_ts($url));
        return $this->UpdateDuneParams($url, Plugin_Constants::HLS);
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        hd_debug_print($movie_id);

        $jfc = $this->get_jfc();
        if ($jfc === null) {
            return null;
        }

        list($item_id, $category_type) = explode('_', $movie_id) + array('', jellyfin_api::MOVIES);
        if (empty($item_id)) {
            hd_debug_print('Real movie ID is empty!');
            return null;
        }

        $movie_item = $jfc->getItemInfo($item_id);
        if (empty($movie_item)) {
            hd_debug_print("Failed to load movie: $item_id from: $category_type");
            return null;
        }

        $movie = new Movie($movie_id, $this->plugin);

        $qualities_str = '';
        $movie_type = safe_get_value($movie_item, 'Type');
        hd_debug_print("movie type: $movie_type", true);
        if ($movie_type === jellyfin_api::MOVIES) {
            $movie_series = $this->create_series($item_id, safe_get_value($movie_item, 'Name', 'no name'));
            $this->fill_series($movie_series, $item_id, safe_get_value($movie_item, 'MediaSources', array()));
            $movie->add_series_data($movie_series);
            $qualities_str = self::get_qualities_str($movie_series);
        } else if ($movie_type === jellyfin_api::SERIES) {
            $seasons = $jfc->getSeasons($item_id);
            $season_idx = 0;
            foreach (safe_get_value($seasons, 'Items', array()) as $season) {
                $season_id = safe_get_value($season, 'Id');
                if (empty($season_id)) continue;

                hd_debug_print("season id: $season_id", true);
                $movie_season = new Movie_Season($season_id, safe_get_value($season, 'IndexNumber', ++$season_idx));
                $season_name = safe_get_value($season, 'Name');
                if (!empty($season_name)) {
                    $movie_season->name = $season_name;
                }
                $movie_season->poster = $jfc->getItemImageUrl($season_id);
                $movie->add_season_data($movie_season);

                $episodes = $jfc->getEpisodes($item_id, $season_id);
                foreach (safe_get_value($episodes, 'Items', array()) as $episode) {
                    $episode_id = safe_get_value($episode, 'Id');
                    if (empty($episode_id)) continue;

                    hd_debug_print("episode id: $episode_id", true);
                    // episodes list contains media sources, older servers may not return them
                    $media_sources = safe_get_value($episode, 'MediaSources');
                    if (empty($media_sources)) {
                        $media_sources = safe_get_value($jfc->getItemInfo($episode_id), 'MediaSources', array());
                    }

                    $movie_series = $this->create_series($episode_id,
                        TR::t('vod_screen_series__1', safe_get_value($episode, 'Name', 'no name')),
                        $season_id
                    );
                    $movie_series->poster = $jfc->getItemImageUrl($episode_id);
                    $movie_series->description = safe_get_value($episode, 'Overview', '');
                    $this->fill_series($movie_series, $episode_id, $media_sources);
                    $movie->add_series_data($movie_series);

                    if (empty($qualities_str)) {
                        $qualities_str = self::get_qualities_str($movie_series);
                    }
                }
            }
        }

        // Director, Actor, Producer, Writer, Editor, Composer
        $persons = array();
        foreach (safe_get_value($movie_item, 'People', array()) as $person) {
            if (!isset($person['Type'], $person['Name'])) continue;

            $persons[$person['Type']][] = $person['Name'];
        }

        // RunTimeTicks are in 100 ns units. Tick counts exceed 32-bit int, json_decode returns them as float
        $length_min = '';
        $ticks = safe_get_value($movie_item, 'RunTimeTicks');
        if (!empty($ticks)) {
            $length_min = (int)round($ticks / 600000000);
        }

        // CriticRating is usually a percentage, but may already be in 10 point (imdb) scale
        $critic_rating = safe_get_value($movie_item, 'CriticRating');
        if (is_numeric($critic_rating) && $critic_rating > 10) {
            $critic_rating = round($critic_rating / 10, 1);
        }

        $details = array();
        if (!empty($qualities_str)) {
            $details[TR::t('vod_screen_quality')] = $qualities_str;
        }

        $movie->set_data(
            safe_get_value($movie_item, 'Name', TR::t('no_title')), // name,
            safe_get_value($movie_item, 'OriginalTitle'), // name_original,
            safe_get_value($movie_item, 'Overview'),  // description,
            $jfc->getItemImageUrl($item_id),  // poster_url,
            $length_min, // length_min,
            safe_get_value($movie_item, 'ProductionYear'), // year,
            implode(', ', safe_get_value($persons, 'Director', array())), // director,
            implode(', ', safe_get_value($persons, 'Writer', array())), // scenario,
            implode(', ', safe_get_value($persons, 'Actor', array())), // actors,
            implode(', ', safe_get_value($movie_item, 'Genres', array())), // genres,
            $critic_rating, // rate_imdb,
            safe_get_value($movie_item, 'CommunityRating', ''), // rate_kinopoisk,
            safe_get_value($movie_item, 'OfficialRating', ''), // rate_mpaa,
            implode(', ', safe_get_value($movie_item, 'ProductionLocations', array())), // country,
            '', // budget
            $details // details
        );

        return $movie;
    }

    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        hd_debug_print(null, true);

        $category_list = array();
        $category_index = array();

        $jfc = $this->get_jfc();
        if ($jfc === null) {
            return false;
        }

        $sources = array(-1 => TR::load('no'));
        foreach (safe_get_value($jfc->getUserViews(), 'Items', array()) as $collection) {
            if (safe_get_value($collection, 'Type') !== "CollectionFolder") continue;

            $id = safe_get_value($collection, 'Id');
            if (empty($id)) continue;

            $collection_type = safe_get_value($collection, 'CollectionType');
            hd_debug_print("Collection type: $collection_type");
            if ($collection_type === jellyfin_api::TVSHOWS_TYPE) {
                $category_type = jellyfin_api::SERIES;
            } else if (empty($collection_type) || $collection_type === jellyfin_api::MOVIES_TYPE) {
                $category_type = jellyfin_api::MOVIES;
            } else {
                continue;
            }

            $items = $jfc->getItems(array('ParentId' => $id, 'Recursive' => 'true', 'IncludeItemTypes' => $category_type, 'StartIndex' => 0, 'Limit' => 1));
            $movie_count = safe_get_value($items, 'TotalRecordCount');
            if (empty($movie_count)) continue;

            $name = safe_get_value($collection, 'Name', 'no name');
            $category = new Vod_Category("{$id}_$category_type", "$name ($movie_count)");
            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
            $sources[$category->get_id()] = $name;
        }

        $exist_filters = array();
        $exist_filters['source'] = array('title' => TR::load('category'), 'values' => $sources);

        $filters = $jfc->getFilters(array('IncludeItemTypes' => jellyfin_api::MOVIES . ',' . jellyfin_api::SERIES, 'Recursive' => 'true'));
        $genres = safe_get_value($filters, 'Genres', array());
        if (!empty($genres)) {
            sort($genres);
            $exist_filters['genre'] = array('title' => TR::load('genre'), 'values' => array(-1 => TR::load('no')) + $genres);
        }

        $years = safe_get_value($filters, 'Years', array());
        if (!empty($years)) {
            rsort($years);
            $exist_filters['years'] = array('title' => TR::load('year'), 'values' => array(-1 => TR::load('no')) + $years);
        }

        $this->set_filter_types($exist_filters);

        hd_debug_print("Categories read: " . count($category_list));
        hd_debug_print("Filters count: " . count($exist_filters));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print("getMovieList: $query_id");

        list($category_id, $category_type) = explode('_', $query_id) + array($query_id, jellyfin_api::MOVIES);
        $query_params['ParentId'] = $category_id;
        $query_params['Recursive'] = 'true';
        $query_params['IncludeItemTypes'] = $category_type === jellyfin_api::SERIES ? jellyfin_api::SERIES : jellyfin_api::MOVIES;
        $query_params['SortBy'] = 'SortName';

        $movies = $this->get_page_movies($query_id, $query_params);
        hd_debug_print("Movies read for query: $query_id: " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print(null, true);
        hd_debug_print("getSearchList: $keyword");

        $query_params['SearchTerm'] = $keyword;
        $query_params['IncludeItemTypes'] = jellyfin_api::MOVIES . ',' . jellyfin_api::SERIES;
        $query_params['Recursive'] = 'true';

        // search list is requested only once, without pagination
        $movies = array();
        $jfc = $this->get_jfc();
        if ($jfc !== null) {
            $query_params['StartIndex'] = 0;
            $query_params['Limit'] = self::PAGE_LIMIT * 4;
            foreach (safe_get_value($jfc->getItems($query_params), 'Items', array()) as $item) {
                $this->CreateShortMovie($item, $movies);
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        return $movies;
    }

    /**
     * @inheritDoc
     */
    public function getFilterList($query_id)
    {
        hd_debug_print(null, true);
        hd_debug_print("getFilterList: $query_id");

        $query_params = array();
        foreach (explode(',', $query_id) as $pair) {
            /** @var array $m */
            if (!preg_match("/^([^:]+):(.+)$/", $pair, $m)) continue;

            $filter = $this->get_filter($m[1]);
            if ($filter === null || empty($filter['values'])) continue;

            $item_key = array_search($m[2], $filter['values']);
            if ($item_key === false || $item_key === -1) continue;

            switch ($m[1]) {
                case 'source':
                    list($query_params['ParentId'], $query_params['IncludeItemTypes']) = explode('_', $item_key) + array('', '');
                    break;
                case 'genre':
                    $query_params['Genres'] = $m[2];
                    break;
                case 'years':
                    $query_params['Years'] = $m[2];
                    break;
            }
        }

        if (empty($query_params)) {
            return array();
        }

        if (empty($query_params['IncludeItemTypes'])) {
            $query_params['IncludeItemTypes'] = jellyfin_api::MOVIES . ',' . jellyfin_api::SERIES;
        }
        $query_params['Recursive'] = 'true';
        $query_params['SortBy'] = 'SortName';

        $movies = $this->get_page_movies($query_id, $query_params);
        hd_debug_print("Movies read for query: $query_id: " . count($movies));
        return $movies;
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * Initialize api and log in. Saved session is reused if it was made with the same server and credentials
     *
     * @return jellyfin_api|null
     */
    protected function get_jfc()
    {
        $vod_url = $this->GetVodListUrl();
        if (empty($vod_url)) {
            hd_debug_print("VOD url not defined");
            return null;
        }

        // server or credentials may be changed in setup
        $hash = hash('crc32', $vod_url . $this->get_login() . $this->get_password());
        if (isset($this->jfc) && $this->session_hash === $hash) {
            return $this->jfc;
        }

        unset($this->jfc);
        $jfc = new jellyfin_api();
        $jfc->init($this->plugin, $vod_url, $this->plugin_info['app_version']);

        // saved session: hash|token|user_id
        list($saved_hash, $token, $user_id) = explode('|', $this->plugin->get_credentials(Ext_Params::M_S_TOKEN)) + array('', '', '');
        if ($saved_hash !== $hash) {
            $token = $user_id = '';
        }

        $access_info = array(
            jellyfin_api::ACCESS_LOGIN => $this->get_login(),
            jellyfin_api::ACCESS_PASSWORD => $this->get_password(),
            jellyfin_api::ACCESS_TOKEN => $token,
            jellyfin_api::ACCESS_USER_ID => $user_id,
        );

        if (!$jfc->login($access_info)) {
            hd_debug_print("Login failed");
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, '');
            return null;
        }

        $session = "$hash|{$jfc->get_access_token()}|{$jfc->get_user_id()}";
        if ($session !== $this->plugin->get_credentials(Ext_Params::M_S_TOKEN)) {
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $session);
        }

        $this->jfc = $jfc;
        $this->session_hash = $hash;
        return $this->jfc;
    }

    /**
     * Load next page of the movies list
     *
     * @param string $page_id
     * @param array $query_params
     * @return array
     */
    protected function get_page_movies($page_id, $query_params)
    {
        $movies = array();

        $start_idx = $this->get_current_page($page_id);
        if ($start_idx < 0) {
            return $movies;
        }

        $jfc = $this->get_jfc();
        if ($jfc === null) {
            return $movies;
        }

        $query_params['StartIndex'] = $start_idx;
        $query_params['Limit'] = self::PAGE_LIMIT;
        $query_params['ImageTypeLimit'] = 1;

        foreach (safe_get_value($jfc->getItems($query_params), 'Items', array()) as $item) {
            $this->CreateShortMovie($item, $movies);
        }

        if (count($movies) < self::PAGE_LIMIT) {
            // last page
            $this->set_next_page($page_id, -1);
        } else {
            $this->get_next_page($page_id, count($movies));
        }

        return $movies;
    }

    /**
     * @param string $id
     * @param string $name
     * @param string $season_id
     * @return Movie_Series
     */
    protected function create_series($id, $name, $season_id = '')
    {
        $movie_series = new Movie_Series($id, $name, MediaURL::encode(array('id' => $id)), $season_id);
        // stream url is made by GetVodStreamUrl
        $movie_series->playback_url_is_stream_url = false;
        return $movie_series;
    }

    /**
     * @param Movie_Series $movie_series
     * @param string $item_id
     * @param array $media_sources
     * @return void
     */
    protected function fill_series($movie_series, $item_id, $media_sources)
    {
        /** @var Movie_Variant[] $qualities */
        $qualities = array();
        foreach ($media_sources as $source) {
            $stream_id = safe_get_value($source, 'Id');
            if (empty($stream_id)) continue;

            // audio track is selected in player: the direct stream contains all tracks of the source
            foreach (safe_get_value($source, 'MediaStreams', array()) as $stream) {
                if (strcasecmp(safe_get_value($stream, 'Type', ''), 'Video') !== 0) continue;

                $q_name = safe_get_value($stream, 'DisplayTitle', safe_get_value($source, 'Name', $stream_id));
                $media_url = MediaURL::encode(array('id' => $item_id, 'stream_id' => $stream_id));
                $quality = new Movie_Variant("{$item_id}_$stream_id", $q_name, $media_url, false);
                $qualities[$q_name] = $quality;
                // default playback url for quality
                if ($stream_id == $item_id) {
                    $qualities['auto'] = $quality;
                }
                break;
            }
        }

        if (count($qualities) > 1) {
            $movie_series->qualities = $qualities;
        }
    }

    /**
     * @param Movie_Series $movie_series
     * @return string
     */
    protected static function get_qualities_str($movie_series)
    {
        if (empty($movie_series->qualities)) {
            return '';
        }

        return implode(', ', array_diff(array_keys($movie_series->qualities), array('auto')));
    }

    /**
     * Extension for the direct stream url. Jellyfin reports container as ffprobe format names,
     * for example "mov,mp4,m4a,3gp,3g2,mj2" or "matroska,webm"
     *
     * @param string $container
     * @return string
     */
    protected static function get_stream_extension($container)
    {
        $formats = explode(',', strtolower($container));
        if (in_array('mp4', $formats)) {
            return 'mp4';
        }
        if (in_array('matroska', $formats) || in_array('mkv', $formats)) {
            return 'mkv';
        }
        if (in_array('mov', $formats)) {
            return 'mov';
        }
        return '';
    }

    /**
     * @param array $movie_info
     * @param array $movies
     * @return void
     */
    protected function CreateShortMovie($movie_info, &$movies)
    {
        $id = safe_get_value($movie_info, 'Id');
        if (empty($id)) {
            return;
        }

        $name = safe_get_value($movie_info, 'Name', 'no name');
        $type = safe_get_value($movie_info, 'Type', jellyfin_api::MOVIES);
        $rating = safe_get_value($movie_info, 'OfficialRating', 0);
        $icon = $this->jfc->getItemImageUrl($id);
        $movie = new Short_Movie("{$id}_$type", $name, $icon, TR::t('vod_screen_movie_info__2', $name, $rating));

        $this->plugin->vod->set_cached_short_movie($movie);

        $movies[] = $movie;
    }
}

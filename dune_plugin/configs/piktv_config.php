<?php
require_once 'lib/default_config.php';

class piktv_config extends default_config
{
    const API_ACTION_MOVIE = 'movie';
    const API_ACTION_SERIAL = 'serial';
    const API_ACTION_SEARCH = 'search';

    const API_COMMAND_GET_VOD = '{API_URL}/xapi10';
    const API_COMMAND_ACCOUNT_INFO = '{API_URL}/xapi10/accountinfo';
    const API_COMMAND_GET_SERVERS = '{API_URL}/xapi10/tv/servers';
    const API_COMMAND_REQUEST_TOKEN = '{API_URL}/auth/';

    /**
     * @inheritDoc
     */
    protected function ensure_token_loaded($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("force request provider token: " . var_export($force, true));

        $token = $this->plugin->get_credentials(Ext_Params::M_S_TOKEN);
        $expired = time() > (int)$this->plugin->get_credentials(Ext_Params::M_EXPIRE_DATA);
        if (!$force && !empty($token) && !$expired) {
            hd_debug_print("request not required", true);
            return true;
        }

        $refresh_token = $this->plugin->get_credentials(Ext_Params::M_R_TOKEN);
        $refresh = $expired && !empty($refresh_token);
        if ($refresh) {
            /*
            grant_type=refresh_token
            refresh_token={{refresh_token}}"
            */
            hd_debug_print("need to refresh token", true);
            $cmd = self::API_COMMAND_REQUEST_TOKEN;
            $pairs['grant_type'] = 'refresh_token';
            $pairs['refresh_token'] = $refresh_token;
        } else {
            /*
            grant_type=password
            username={LOGIN}"
            password={PASSWORD}
            */
            hd_debug_print("need to request token", true);
            $cmd = self::API_COMMAND_REQUEST_TOKEN;
            $pairs['grant_type'] = 'password';
            $pairs['username'] = $this->get_login();
            $pairs['password'] = $this->get_password();
        }

        $curl_opt[CURLOPT_POST] = true;
        $curl_opt[CURLOPT_HTTPHEADER][] = "Content-Type: application/x-www-form-urlencoded";
        $data = '';
        foreach($pairs as $key => $value) {
            if (!empty($data)) {
                $data .= "&";
            }
            $data .= $key . "=" . urlencode($value);
        }
        $curl_opt[CURLOPT_POSTFIELDS] = $data;

        $data = $this->execApiCommand($cmd, null, true, $curl_opt);
        if (isset($data->access_token)) {
            hd_debug_print("token requested", true);
            $expired = time() + $data->expires_in;
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, $data->access_token);
            $this->plugin->set_credentials(Ext_Params::M_R_TOKEN, $data->refresh_token);
            $this->plugin->set_credentials(Ext_Params::M_EXPIRE_DATA, $expired);
            hd_debug_print("s_token: $data->access_token", true);
            hd_debug_print("r_token: $data->refresh_token", true);
            hd_debug_print("expired: $expired", true);
            return true;
        }

        if ($refresh) {
            hd_debug_print("token not refreshed, make full request: " . json_encode($data), true);
            $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, '');
            $this->plugin->set_credentials(Ext_Params::M_R_TOKEN, '');
            return $this->ensure_token_loaded(true);
        }

        hd_debug_print("token not received: " . pretty_json_format($data), true);
        return false;
    }

    /**
     * @inheritDoc
     */
    public function get_servers()
    {
        hd_debug_print(null, true);

        if (empty($this->servers)) {
            $response = $this->execApiCommand(self::API_COMMAND_GET_SERVERS);
            hd_debug_print("GetServers: " . pretty_json_format($response), true);
            if (isset($response->data)) {
                foreach ($response->data as $server) {
                    $this->servers[(string)$server->id] = $server->title;
                }
            }
        }

        return $this->servers;
    }

    /**
     * @inheritDoc
     */
    public function GetAccountInfo($force = false)
    {
        hd_debug_print(null, true);
        hd_debug_print("force get_provider_info: " . var_export($force, true), true);

        if (!$this->ensure_token_loaded()) {
            hd_debug_print("Failed to get provider token", true);
            return null;
        }

        if ((empty($this->account_info) || $force)) {
            $account_info = $this->execApiCommand(self::API_COMMAND_ACCOUNT_INFO);
            if ($account_info === false || isset($account_info->error)) {
                hd_debug_print("Failed to get provider token", true);
                $this->plugin->set_credentials(Ext_Params::M_S_TOKEN, '');
                $this->plugin->set_credentials(Ext_Params::M_R_TOKEN, '');
            } else {
                hd_debug_print("get_provider_info: " . pretty_json_format($account_info), true);
                $this->account_data = $account_info;
            }
        }

        return $this->account_data;
    }

    /**
     * @inheritDoc
     */
    public function TryLoadMovie($movie_id)
    {
        hd_debug_print(null, true);
        // movies_84636 or serials_84636
        hd_debug_print("TryLoadMovie: $movie_id");
        $arr = explode("_", $movie_id);
        $id = isset($arr[1]) ? $arr[1] : $movie_id;

        $json = $this->make_json_request("/video/$id");

        if ($json === false || $json === null) {
            hd_debug_print("failed to load movie: $movie_id");
            return null;
        }

        $movie = new Movie($movie_id, $this->plugin);
        $movieData = $json->data;
        if (isset($movieData->seasons)) {
            // collect series
            foreach ($movieData->seasons as $season) {
                $movie->add_season_data($season->id,
                    empty($season->name)
                        ? TR::t('vod_screen_season__1', $season->number)
                        : $season->name,
                    '');

                foreach ($season->series as $episode) {
                    hd_debug_print("movie playback_url: {$episode->files['url']}");

                    $movie->add_series_with_variants_data(
                        $episode->id,
                        TR::t('vod_screen_series__1', $episode->number),
                        $episode->name,
                        array(),
                        array(),
                        $episode->files[0]->url,
                        $season->id);
                }
            }
        } else {
            hd_debug_print("movie playback_url: {$movieData->files[0]->url}");
            $movie->add_series_data($movie_id, $movieData->name, '', $movieData->files[0]->url);
        }

        $movie->set_data(
            $movieData->name,
            $movieData->original_name,
            $movieData->description,
            $movieData->poster,
            $movieData->time,
            $movieData->year,
            $movieData->director,
            '',
            $movieData->actors,
            self::collect_genres($movieData),
            $movieData->rating,
            '',
            '',
            $movieData->country
        );

        return $movie;
    }


    /**
     * @inheritDoc
     */
    public function fetchVodCategories(&$category_list, &$category_index)
    {
        $jsonItems = $this->make_json_request("/cat");
        if ($jsonItems === false || isset($jsonItems->error)) {
            return false;
        }

        $category_list = array();
        $category_index = array();

        foreach ($jsonItems->data as $node) {
            $id = (string)$node->id;
            $category = new Vod_Category($id, "$node->name ($node->count)");

            // fetch genres for category
            $genres = $this->make_json_request("/cat/$id/genres");
            if ($genres === false) {
                continue;
            }

            $gen_arr = array();
            if (isset($genres->data)) {
                foreach ($genres->data as $genre) {
                    $gen_arr[] = new Vod_Category((string)$genre->id, "$genre->title ($genre->count)", $category);
                }
            }

            $category->set_sub_categories($gen_arr);

            $category_list[] = $category;
            $category_index[$category->get_id()] = $category;
        }

        hd_debug_print("Categories read: " . count($category_list));
        return true;
    }

    /**
     * @inheritDoc
     */
    public function getSearchList($keyword)
    {
        hd_debug_print("getSearchList $keyword");
        $keyword = urlencode($keyword);
        $searchRes = $this->make_json_request("/filter/by_name?name=$keyword&page=1&per_page=999999999");

        return ($searchRes === false) ? array() : $this->CollectSearchResult($searchRes, $searchRes);
    }

    /**
     * @inheritDoc
     */
    public function getMovieList($query_id)
    {
        hd_debug_print($query_id);
        $this->get_next_page($query_id);
        $arr = explode("_", $query_id);
        $genre_id = isset($arr[1]) ? $arr[1] : $query_id;

        $response = $this->make_json_request("/genres/$genre_id?page=1&per_page=999999999");
        return $response === false ? array() : $this->CollectSearchResult($query_id, $response);
    }

    /**
     * @param string $query_id
     * @param Object $json
     * @return array
     */
    protected function CollectSearchResult($query_id, $json)
    {
        $movies = array();

        $page_id = $query_id;
        $current_idx = $this->get_current_page($page_id);
        if ($current_idx < 0)
            return $movies;

        foreach ($json->data as $entry) {
            $genresArray = array();
            if (isset($entry->genres)) {
                foreach ($entry->genres as $genre) {
                    $genresArray[] = $genre->title;
                }
            }
            if (isset($entry->name)) {
                $genre_str = implode(", ", $genresArray);
                $movies[] = new Short_Movie(
                    $entry->id,
                    $entry->name,
                    $entry->poster,
                    TR::t('vod_screen_movie_info__5', $entry->name, $entry->year, $entry->country, $genre_str, $entry->rating)
                );
            }
        }

        hd_debug_print("Movies found: " . count($movies));
        $this->set_next_page($page_id, -1);

        return $movies;
    }

    /**
     * @param string|null $path
     * @return bool|object
     */
    protected function make_json_request($path = null)
    {
        if (!$this->ensure_token_loaded()) {
            return false;
        }

        $curl_opt[CURLOPT_CUSTOMREQUEST] = $path;

        $curl_opt[CURLOPT_HTTPHEADER] = array(
            "Content-Type: application/json; charset=utf-8",
            "Authorization: Bearer " . $this->plugin->get_credentials(Ext_Params::M_S_TOKEN)
        );

        $jsonItems = $this->execApiCommand(self::API_COMMAND_GET_VOD, null, true, $curl_opt);
        if ($jsonItems === false) {
            $exception_msg = TR::load_string('err_load_vod') . "\n\n" . $this->curl_wrapper->get_raw_response_headers();
            hd_debug_print($exception_msg);
            return false;
        }

        return $jsonItems;
    }

    protected static function collect_genres($entry)
    {
        $genres_str = '';
        if (isset($entry->genres)) {
            $genres = array();
            foreach ($entry->genres as $genre) {
                if (!empty($genre)) {
                    $genres[] = $genre->title;
                }
            }
            $genres_str = implode(", ", $genres);
        }

        return $genres_str;
    }
}

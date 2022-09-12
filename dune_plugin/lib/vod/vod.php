<?php

interface Vod
{
    /**
     * @param string $movie_id
     * @param $plugin_cookies
     */
    public function try_load_movie($movie_id, &$plugin_cookies);

    /**
     * @param string $movie_id
     * @return bool
     */
    public function is_failed_movie_id($movie_id);

    /**
     * @return bool
     */
    public function is_movie_page_supported();

    /**
     * @return bool
     */
    public function is_favorites_supported();

    /**
     * @param $movie_id
     * @return Movie|null
     */
    public function get_cached_movie($movie_id);

    public function clear_movie_cache();

    /**
     * @param MediaURL $media_url
     * @param $plugin_cookies
     * @return array|null
     */
    public function get_vod_info(MediaURL $media_url, &$plugin_cookies);

    /**
     * @param string $playback_url
     * @param $plugin_cookies
     * @return string
     */
    public function get_vod_stream_url($playback_url, &$plugin_cookies);

    /**
     * @return array|null
     */
    public function get_genre_ids();

    /**
     * @param string $genre_id
     * @return string|null
     */
    public function get_genre_caption($genre_id);

    /**
     * @param string $genre_id
     * @return string|null
     */
    public function get_genre_icon_url($genre_id);

    /**
     * @param $genre_id
     * @return false|string|null
     */
    public function get_genre_media_url_str($genre_id);

    public function clear_genre_cache();

    /**
     * @return array|null
     */
    public function get_vod_list_folder_views();

    /**
     * @return array|null
     */
    public function get_vod_genres_folder_views();

    /**
     * @param MediaURL $media_url
     * @return mixed
     */
    public function get_archive(MediaURL $media_url);
}

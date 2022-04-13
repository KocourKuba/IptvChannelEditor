<?php

interface Group
{
    /**
     * @return string
     */
    public function get_id();

    /**
     * @return string
     */
    public function get_title();

    /**
     * @return string
     */
    public function get_icon_url();

    /**
     * @return bool
     */
    public function is_favorite_channels();

    /**
     * @return bool
     */
    public function is_all_channels();

    /**
     * @param $plugin_cookies
     * @return Hashed_Array
     */
    public function get_channels(&$plugin_cookies);
}

<?php
require_once 'default_group.php';

class Favorites_Group extends Default_Group
{
    /**
     * @var Tv
     */
    private $tv;

    /**
     * @param Tv $tv
     * @param string $id
     * @param string $title
     * @param string $icon_url
     */
    public function __construct(Tv $tv, $id, $title, $icon_url)
    {
        parent::__construct($id, $title, $icon_url);

        $this->tv = $tv;
    }

    /**
     * @return bool
     */
    public function is_favorite_channels()
    {
        return true;
    }

    /**
     * @param $plugin_cookies
     * @return Hashed_Array
     * @throws Exception
     */
    public function get_channels(&$plugin_cookies)
    {
        $channels = new Hashed_Array;

        $fav_channel_ids = $this->tv->get_fav_channel_ids($plugin_cookies);
        foreach ($fav_channel_ids as $channel_id) {
            $channels->put($this->tv->get_channel($channel_id));
        }

        return $channels;
    }
}

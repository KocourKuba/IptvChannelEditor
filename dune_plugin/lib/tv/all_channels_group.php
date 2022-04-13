<?php
require_once 'default_group.php';

class All_Channels_Group extends Default_Group
{
    /**
     * @var Tv
     */
    private $tv;

    /**
     * @param Tv $tv
     * @param string $title
     * @param string $icon_url
     */
    public function __construct(Tv $tv, $title, $icon_url)
    {
        parent::__construct($tv->get_all_channel_group_id(), $title, $icon_url);

        $this->tv = $tv;
    }

    /**
     * @return bool
     */
    public function is_all_channels()
    {
        return true;
    }

    /**
     * @param $plugin_cookies
     * @return Hashed_Array|Channel[]
     */
    public function get_channels(&$plugin_cookies)
    {
        return $this->tv->get_channels();
    }
}

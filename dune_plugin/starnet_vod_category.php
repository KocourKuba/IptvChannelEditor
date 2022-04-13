<?php

class Starnet_Vod_Category
{
    const DEFAULT_ICON = 'plugin_file://icons/movie_folder.png';

    /**
     * @var string
     */
    private $id;

    /**
     * @var string
     */
    private $caption;

    /**
     * @var string
     */
    private $icon_url;

    /**
     * @var array
     */
    private $sub_categories;

    /**
     * @var Starnet_Vod_Category|null
     */
    private $parent;

    /**
     * @var string |null
     */
    private $url;

    /**
     * @param string $id
     * @param string $caption
     * @param Starnet_Vod_Category|null $parent
     * @param string|null $url
     */
    public function __construct($id, $caption, $parent = null, $url = null)
    {
        $this->id = $id;
        $this->caption = $caption;
        $this->icon_url = self::DEFAULT_ICON;
        $this->parent = $parent;
        $this->url = $url;
    }

    /**
     * @return string
     */
    public function get_id()
    {
        return $this->id;
    }

    /**
     * @return string
     */
    public function get_caption()
    {
        return $this->caption;
    }

    /**
     * @return string
     */
    public function get_icon_path()
    {
        return $this->icon_url;
    }

    /**
     * @param $arr
     */
    public function set_sub_categories($arr)
    {
        $this->sub_categories = $arr;
    }

    /**
     * @return array
     */
    public function get_sub_categories()
    {
        return $this->sub_categories;
    }

    /**
     * @return Starnet_Vod_Category|null
     */
    public function get_parent()
    {
        return $this->parent;
    }

    /**
     * @return string|null
     */
    public function get_url()
    {
        return $this->url;
    }
}

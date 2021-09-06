<?php

class StarnetVodCategory
{
    const DEFAULT_ICON = 'plugin_file://icons/movie_folder.png';
    private $id;
    private $caption;
    private $icon_url;

    private $sub_categories;

    public function __construct($id, $caption)
    {
        $this->id = $id;
        $this->caption = $caption;
        $this->icon_url = StarnetVodCategory::DEFAULT_ICON;
        $this->sub_categories = null;
    }

    public function get_id()
    {
        return $this->id;
    }

    public function get_caption()
    {
        return $this->caption;
    }

    public function get_icon_path()
    {
        return $this->icon_url;
    }

    public function set_sub_categories($arr)
    {
        $this->sub_categories = $arr;
    }

    public function get_sub_categories()
    {
        return $this->sub_categories;
    }
}

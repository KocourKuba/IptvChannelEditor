<?php
///////////////////////////////////////////////////////////////////////////

class DemoVodCategory
{
    private $id;
    private $caption;
    private $icon_url;

    private $sub_categories;

    public function __construct($id, $caption, $icon_url)
    {
        $this->id = $id;
        $this->caption = $caption;
        $this->icon_url = $icon_url;
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

///////////////////////////////////////////////////////////////////////////
?>

<?php
require_once 'lib/default_group.php';

class Vod_Group extends Default_Group
{
    /**
     * @param $id
     * @param string $title
     * @param string $icon_url
     */
    public function __construct($id, $title, $icon_url)
    {
        parent::__construct($id, $title, $icon_url);

        $this->is_vod_group = true;
    }
}

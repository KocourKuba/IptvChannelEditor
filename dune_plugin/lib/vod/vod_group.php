<?php
require_once 'lib/default_group.php';

class Vod_Group extends Default_Group
{
    /**
     * @return bool
     */
    public function is_vod_group()
    {
        return true;
    }
}

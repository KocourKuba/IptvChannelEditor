<?php
require_once 'lib/default_group.php';

class Favorites_Group extends Default_Group
{
    /**
     * @return bool
     */
    public function is_favorite_group()
    {
        return true;
    }
}

<?php

class Movie_Season
{
    /**
     * @var string
     */
    public $id;

    /**
     * @var string
     */
    public $name = '';

    /**
     * @var string
     */
    public $season_url = '';

    /**
     * @var string
     */
    public $type = '';

    /**
     * @param string $id
     */
    public function __construct($id)
    {
        if (is_null($id)) {
            hd_print("Movie_Season::id is not set");
        }
        $this->id = (string)$id;
    }
}

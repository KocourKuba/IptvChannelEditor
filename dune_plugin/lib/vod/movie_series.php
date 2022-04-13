<?php

class Movie_Series
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
    public $playback_url = '';

    /**
     * @var bool
     */
    public $playback_url_is_stream_url = true;

    /**
     * @param string $id
     * @throws Exception
     */
    public function __construct($id)
    {
        if (is_null($id)) {
            throw new Exception("Movie_Series::id is null");
        }

        $this->id = (string)$id;
    }
}

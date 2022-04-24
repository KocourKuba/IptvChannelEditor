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
     * @param $id string
     * @param $name string
     * @param $playback_url string
     * @param $playback_url_is_stream_url bool
     * @throws Exception
     */
    public function __construct($id, $name, $playback_url, $playback_url_is_stream_url)
    {
        if (is_null($id)) {
            throw new Exception("Movie_Series::id is null");
        }

        $this->id = $id;
        $this->name = $name;
        $this->playback_url = $playback_url;
        $this->playback_url_is_stream_url = $playback_url_is_stream_url;
    }
}

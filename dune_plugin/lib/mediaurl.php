<?php

class MediaURL
{
    // Original media-url string.
    private $str;

    // If media-url string contains map, it's decoded here.
    // Null otherwise.
    private $map;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $str string
     * @param $map|null array
     */
    private function __construct($str, $map)
    {
        $this->str = $str;
        $this->map = $map;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param $key
     * @param $value
     */
    public function __set($key, $value)
    {
        if (is_null($this->map)) {
            $this->map = (object)array();
        }

        $this->map->{$key} = $value;
    }

    /**
     * @param $key
     */
    public function __unset($key)
    {
        if (is_null($this->map)) {
            return;
        }

        unset($this->map->{$key});
    }

    /**
     * @param $key
     * @return mixed|null
     */
    public function __get($key)
    {
        if (is_null($this->map)) {
            return null;
        }

        return isset($this->map->{$key}) ? $this->map->{$key} : null;
    }

    /**
     * @param $key
     * @return bool
     */
    public function __isset($key)
    {
        if (is_null($this->map)) {
            return false;
        }

        return isset($this->map->{$key});
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * @return string
     */
    public function get_raw_string()
    {
        return $this->str;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * @param array $m
     * @return false|string
     */
    public static function encode($m)
    {
        return json_encode($m);
    }

    ///////////////////////////////////////////////////////////////////////

    /**
     * @param string $s
     * @return MediaURL
     */
    public static function decode($s)
    {
        if (strpos($s, '{') !== 0) {
            return new MediaURL($s, null);
        }

        return new MediaURL($s, json_decode($s));
    }
}

<?php

class Hashed_Array implements Iterator
{
    private $seq;
    private $map;

    private $pos;

    public function __construct()
    {
        $this->seq = array();
        $this->map = array();

        $this->pos = 0;
    }

    public function size()
    {
        return count($this->seq);
    }

    public function get_by_ndx($ndx)
    {
        return $this->seq[$ndx];
    }

    public function put($o)
    {
        $id = $o->get_id();
        if ($this->has($o->get_id())) {
            hd_print("dup: $id");
        } else {
            $this->seq[] = $o;
            $this->map[$id] = $o;
        }
    }

    public function get($key)
    {
        return HD::get_map_element($this->map, $key);
    }

    public function has($key)
    {
        return isset($this->map[$key]);
    }

    public function usort($callback_name)
    {
        usort($this->seq, $callback_name);
    }

    public function rewind()
    {
        $this->pos = 0;
    }

    public function current()
    {
        return $this->seq[$this->pos];
    }

    public function key()
    {
        return $this->pos;
    }

    public function next()
    {
        ++$this->pos;
    }

    public function valid()
    {
        return $this->pos < count($this->seq);
    }
}

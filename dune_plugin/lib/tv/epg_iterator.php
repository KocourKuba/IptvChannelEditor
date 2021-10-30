<?php

class EpgIterator implements Iterator
{
    private $_epg;
    private $_from;
    private $_till;

    private $_pos;
    private $_depleted = false;

    public function __construct($epg, $from, $till)
    {
        $this->_epg = $epg;
        $this->_from = $from;
        $this->_till = $till;

        $this->rewind();
    }

    public function rewind()
    {
        $this->_pos = -1;
        $this->_depleted = false;
        $this->next();
    }

    public function current()
    {
        return $this->valid() ? $this->_epg[$this->_pos] : null;
    }

    public function key()
    {
        if (!$this->valid()) {
            return null;
        }

        return $this->_pos;
    }

    public function next()
    {
        if (!$this->valid()) {
            return;
        }

        $found = 0;

        for ($i = $this->_pos + 1, $iMax = count($this->_epg); $i < $iMax; ++$i) {
            $t = $this->_epg[$i]->get_start_time();

            if ($this->_from <= $t && $t <= $this->_till) {
                $this->_pos = $i;
                $found = 1;
                break;
            }
        }

        if (!$found) {
            $this->_depleted = true;
        }
    }

    public function valid()
    {
        return !$this->_depleted;
    }
}

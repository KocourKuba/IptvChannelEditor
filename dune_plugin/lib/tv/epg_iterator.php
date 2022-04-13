<?php

class Epg_Iterator implements Iterator
{
    /**
     * @var Default_Epg_Item[]
     */
    private $_epg;

    /**
     * @var int
     */
    private $_from;

    /**
     * @var int
     */
    private $_till;

    /**
     * @var int
     */
    private $_pos;

    /**
     * @var bool
     */
    private $_depleted = false;

    /**
     * @param Default_Epg_Item[] $epg
     * @param int $from
     * @param int $till
     */
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

    /**
     * @return Default_Epg_Item|null
     */
    public function current()
    {
        return $this->valid() ? $this->_epg[$this->_pos] : null;
    }

    /**
     * @return int|null
     */
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

    /**
     * @return bool
     */
    public function valid()
    {
        return !$this->_depleted;
    }
}

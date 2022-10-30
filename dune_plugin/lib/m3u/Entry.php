<?php

require_once 'ExtInf.php';
require_once 'ExtGrp.php';

class Entry
{
    /**
     * @var ExtInf|null
     */
    private $extInf;

    /**
     * @var ExtGrp|null
     */
    private $extGrp;

    /**
     * @var string
     */
    private $path;

    /**
     * @return ExtInf|null
     */
    public function getExtInf()
    {
        return $this->extInf;
    }

    /**
     * @param ExtInf $extInf
     * @return Entry
     */
    public function setExtInf(ExtInf $extInf)
    {
        $this->extInf = $extInf;
        return $this;
    }

    /**
     * @return ExtGrp|null
     */
    public function getExtGrp()
    {
        return $this->extGrp;
    }

    /**
     * @param ExtGrp $extGrp
     * @return Entry
     */
    public function setExtGrp(ExtGrp $extGrp)
    {
        $this->extGrp = $extGrp;
        return $this;
    }

    /**
     * @return string
     */
    public function getPath()
    {
        return $this->path;
    }

    /**
     * @param string $path
     * @return Entry
     */
    public function setPath($path)
    {
        $this->path = $path;
        return $this;
    }

    /**
     * @return string
     */
    public function getTitle()
    {
        return $this->extInf !== null ? $this->extInf->getTitle() : '';
    }

    /**
     * @return string
     */
    public function getGroupTitle()
    {
        $group_title = $this->extGrp !== null ? $this->extGrp->getGroup() : '';
        if (empty($group_title)) {
            $group_title = $this->findAttribute('group-title');
        }

        return $group_title;
    }

    /**
     * @param $attr
     * @return string
     */
    public function findAttribute($attr)
    {
        if ($this->extInf === null)
            return '';

        $attributes = $this->extInf->getAttributes();

        return $attributes !== null ? $attributes->getAttribute($attr) : '';
    }
}

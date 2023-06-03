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
     * @var string
     */
    private $group_title;

    /**
     * @var string
     */
    private $parsed_title;

    /**
     * @return ExtInf|null
     */
    public function getExtInf()
    {
        return $this->extInf;
    }

    /**
     * @param ExtInf $extInf
     */
    public function setExtInf(ExtInf $extInf)
    {
        $this->extInf = $extInf;
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
     */
    public function setExtGrp(ExtGrp $extGrp)
    {
        $this->extGrp = $extGrp;
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
     */
    public function setPath($path)
    {
        $this->path = $path;
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
        if (empty($this->group_title)) {
            $group_title = ($this->extGrp !== null) ? $this->extGrp->getGroup() : '';
            if (empty($group_title)) {
                $this->group_title = $this->getAttribute('group-title');
                if (empty($this->group_title) || $this->group_title === "null") {
                    $this->group_title = TR::t('no_category');
                }
            }
        }
        return $this->group_title;
    }

    /**
     * @param $attr
     * @return string
     */
    public function getAttribute($attr)
    {
        if ($this->extInf === null)
            return '';

        return $this->extInf->getAttribute($attr);
    }
}

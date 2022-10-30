<?php
require_once 'TagAttributes.php';

class ExtInf
{
    /**
     * @var string
     */
    private $title;

    /**
     * @var TagAttributes
     */
    private $attributes;

    /**
     * @param string $line
     */
    public function __construct($line)
    {
        $this->makeData($line);
    }

    /**
     * EXTINF format:
     * #EXTINF:<duration> [<attributes-list>], <title>
     * example:
     * #EXTINF:-1 tvg-name="Первый HD" tvg-logo="http://server/logo/1.jpg" group-title="Эфирные каналы",Первый канал HD
     *
     * @param string $line
     */
    protected function makeData($line)
    {
        $tmp = substr($line, 8); // #EXTINF:
        $split = explode(',', $tmp, 2);
        $this->setTitle(trim($split[1]));
        $this->setAttributes(new TagAttributes($split[0]));
    }

    /**
     * @param string $title
     */
    public function setTitle($title)
    {
        $this->title = $title;
    }

    /**
     * @return string
     */
    public function getTitle()
    {
        return $this->title;
    }

    /**
     * @param TagAttributes $attributes
     */
    public function setAttributes($attributes)
    {
        $this->attributes = $attributes;
    }

    /**
     * @return TagAttributes
     */
    public function getAttributes()
    {
        return $this->attributes;
    }
}

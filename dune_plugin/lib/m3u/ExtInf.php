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
        $this->parseData($line);
    }

    /**
     * EXTINF format:
     * #EXTINF:<duration> [<attributes-list>], <title>
     * example:
     * #EXTINF:-1 tvg-name="Первый HD" tvg-logo="http://server/logo/1.jpg" group-title="Эфирные каналы",Первый канал HD
     *
     * @param string $line
     */
    protected function parseData($line)
    {
        $tmp = substr($line, 8); // #EXTINF:
        $split = explode(',', $tmp, 2);
        $this->setTitle(trim($split[1]));
        $this->attributes = new TagAttributes($split[0]);
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
     * @return TagAttributes
     */
    public function getAttributes()
    {
        return $this->attributes;
    }

    /**
     * @param string $name
     * @return string
     */
    public function getAttribute($name)
    {
        return empty($this->attributes) ? '' : $this->attributes->getAttribute($name);
    }
}

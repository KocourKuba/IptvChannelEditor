<?php
require_once 'utils.php';

class EpgParser
{
    //	Source data
    private $file;
    private $content;

    //	for temporary file if from content or from url.
    private $isTemp;
    public $temp_dir = '/tmp';    //unix, change this for windows.

    //	channel settings
    private $channels;
    private $channels_groupby = '@id';

    //	programmes settings.
    private $epg_data;
    private $epg_data_groupby = '@id';

    //	filter
    private $channelFilter = array();
    private $ignoreDescr = array();

    //	zone.
    private $targetTimeZone;

    public function __construct()
    {
        $this->targetTimeZone = date_default_timezone_get();
    }

    /**
     * @param mixed $file
     */
    public function setFile($file)
    {
        $this->file = $file;
    }

    /**
     * @param mixed $content = xml parsed string.
     */
    public function setContent($content)
    {
        $this->content = $content;
    }

    /**
     * @param mixed $channelFilter
     */
    public function setChannelFilter($channelFilter)
    {
        $this->channelFilter[$channelFilter] = 1;
    }

    /**
     * @param string $descr
     */
    public function setIgnoreDescr($descr)
    {
        $this->ignoreDescr[$descr] = 1;
    }

    /**
     * @param mixed $targetTimeZone
     */
    public function setTargetTimeZone($targetTimeZone)
    {
        $this->targetTimeZone = $targetTimeZone;
    }

    /**
     * Set group by for channels must be channels attribute.
     * @param $group - channel will be grouped with. must be @id or param attribute.
     */
    public function setChannelGroup($group)
    {
        $this->channels_groupby = $group;
    }

    /**
     * Set group by for channels must be channels attribute.
     * @id = array index starting from 0
     * @param $group - programmes will be grouped with. must be @id or param attribute.
     */
    public function setProgramGroup($group)
    {
        $this->epg_data_groupby = $group;
    }

    /**
     * Parse the date from string in possible formats.
     */
    public function getDate($date)
    {

        try {
            $dt = DateTime::createFromFormat('YmdHis P', $date, new DateTimeZone('UTC'));
            $dt->setTimezone(new DateTimeZone($this->targetTimeZone));
            return $dt->format('Y-m-d H:i:s');
        } catch (Exception $e) {
        }

        try {
            $dt = DateTime::createFromFormat('YmdHis', $date, new DateTimeZone('UTC'));
            $dt->setTimezone(new DateTimeZone($this->targetTimeZone));
            return $dt->format('Y-m-d H:i:s');
        } catch (Exception $e) {
        }

        try {
            $ex = explode(' ', $date);
            $sd = $ex[0];
            $ed = $ex[1];

            if (strlen($sd) == 13) {
                $sd = "{$sd}0";
            }

            $date = $sd . " " . $ed;

            $dt = DateTime::createFromFormat('YmdHis P', $date, new DateTimeZone('UTC'));
            $dt->setTimezone(new DateTimeZone($this->targetTimeZone));
            return $dt->format('Y-m-d H:i:s');
        } catch (Exception $e) {
        }

        return null;
    }

    /**
     * @param $descr
     *
     * @return string
     */
    private function filterDescr($descr)
    {
        if (array_key_exists($descr, $this->ignoreDescr)) {
            return '';
        }
        return $descr;
    }

    /**
     * @return mixed
     */
    public function getChannels()
    {
        return $this->channels;
    }

    /**
     * @return array
     */
    public function getEpgData()
    {
        return $this->epg_data;
    }

    /**
     *
     */
    public function resetChannelFilter()
    {
        $this->channelFilter = array();
    }

    /**
     *
     */
    private function channelMatchFilter($channel)
    {
        return array_key_exists($channel, $this->channelFilter);
    }

    /**
     * @throws Exception
     */
    public function parseChannels()
    {
        $xml = new XMLReader();
        $xml->open($this->getXmlFile());

        /** @noinspection PhpStatementHasEmptyBodyInspection */
        /** @noinspection LoopWhichDoesNotLoopInspection */
        /** @noinspection MissingOrEmptyGroupStatementInspection */
        while ($xml->read() && $xml->name !== 'channel') {
        }

        $i = 0;
        while ($xml->name === 'channel') {
            $element = new SimpleXMLElement($xml->readOuterXML());

            $group_by = $this->channels_groupby === '@id' ? (@$i++) : strval($element->attributes()->{$this->epg_data_groupby});

            //	se the id
            $channel_id = $group_by ?: 1;
            if (!count($this->channelFilter) || $this->channelMatchFilter(strval($element->attributes()->channel))) {
                $this->channels[$channel_id] = array(
                    'id' => strval($element->attributes()->id),
                    'display-name' => strval($element->{'display-name'}),
                    'url' => strval($element->{'url'}),
                    'email' => strval($element->{'email'}),
                    'icon' => null,
                );

                if (isset($element->{'icon'}) && $element->{'icon'} instanceof SimpleXMLElement) {
                    $attributes = $element->{'icon'}->attributes();
                    $path_info = pathinfo($attributes->src);

                    if (empty($attributes->src)) {
                        $this->channels[$channel_id]['icon'] = strval($element->{'icon'});
                        $xml->next('channel');
                        unset($element);
                    } elseif (!filter_var($attributes->src, FILTER_VALIDATE_URL)) {
                        $this->channels[$channel_id]['icon'] = strval($element->{'icon'});
                        $xml->next('channel');
                        unset($element);
                    } elseif (empty($path_info['extension'])) {
                        $this->channels[$channel_id]['icon'] = strval($element->{'icon'});
                        $xml->next('channel');
                        unset($element);
                        continue;
                    } else {
                        $this->channels[$channel_id]['icon'] = strval($attributes->src);
                    }
                }
            }

            $xml->next('channel');
            unset($element);
        }

        $xml->close();
    }

    /**
     * @throws Exception
     */
    public function parseEpg()
    {
        $xml = new XMLReader();
        $xml->open($this->getXmlFile());

        /** @noinspection PhpStatementHasEmptyBodyInspection */
        /** @noinspection LoopWhichDoesNotLoopInspection */
        /** @noinspection MissingOrEmptyGroupStatementInspection */
        while ($xml->read() && $xml->name !== 'programme') {
        }

        $i = 0;
        while ($xml->name === 'programme') {
            $element = new SimpleXMLElement($xml->readOuterXML());
            //hd_print("read: " . strval($element->attributes()->channel));
            if (!count($this->channelFilter) || $this->channelMatchFilter(strval($element->attributes()->channel))) {
                $startString = $this->getDate(strval($element->attributes()->start));
                $stopString = $this->getDate(strval($element->attributes()->stop));
                $grouper = $this->epg_data_groupby === '@id' ? (@$i++) : strval($element->attributes()->{$this->epg_data_groupby});
                $this->epg_data[$grouper ?: 0] = array(
                    'start' => $startString,
                    'start_raw' => strval($element->attributes()->start),
                    'channel' => strval($element->attributes()->channel),
                    'stop' => $stopString,
                    'title' => strval($element->title),
                    'sub-title' => strval($element->{'sub-title'}),
                    'desc' => $this->filterDescr(strval($element->desc)),
                    'date' => intval($element->date),
                    'category' => strval($element->category),
                    'credits' => strval($element->credits),
                    'country' => strval($element->country),
                    'icon' => strval($element->icon),
                    'episode-num' => strval($element->{'episode-num'}),
                );
                //hd_print("found: " . strval($element->title) . strval($element->desc));
            }

            $xml->next('programme');
            unset($element);
        }

        $xml->close();
    }

    /**
     * @throws Exception
     *
     * @return string
     */
    protected function getXmlFile()
    {
        if (!$this->file) {
            throw new Exception('missing file: please use setFile before parse');
        }

        if (!file_exists($this->file)) {
            throw new Exception('file does not exists: ' . $this->file);
        }

        $xml_file = '';
        preg_match('|^.*\/.*(\..+)$|', $this->file, $match);
        if ($match[1] === '.gz')
            $xml_file = 'compress.zlib://';

        $xml_file .= $this->file;
        return $xml_file;
    }
}

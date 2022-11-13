<?php

require_once 'Entry.php';

class M3uParser
{
    /**
     * @var string
     */
    private $file_name;

    /**
     * @var SplFileObject
     */
    private $m3u_file;

    /**
     * @param $file_name
     */
    public function setupParser($file_name)
    {
        $this->m3u_file = null;
        $this->file_name = $file_name;

        try {
            $file = new SplFileObject($file_name);
        } catch (Exception $ex) {
            hd_print("Can't read file.");
            return;
        }

        $file->setFlags(SplFileObject::DROP_NEW_LINE);

        $this->m3u_file = $file;
    }

    /**
     * Parse m3u by seeks file, slower and
     * less memory consumption for large m3u files
     * But still may cause memory exhausting
     *
     * @return Entry[]
     */
    public function parseFile()
    {
        $data = array();
        if ($this->m3u_file === null) {
            hd_print("parseFile: Bad file");
            return $data;
        }

        $this->m3u_file->rewind();

        $t = microtime(1);
        $entry = new Entry();
        foreach($this->m3u_file as $line) {
            if (!$this->parseLine($line, $entry)) continue;

            $data[] = $entry;
            $entry = new Entry();
        }

        hd_print("parseFile " . (microtime(1) - $t) . " secs");
        return $data;
    }

    /**
     * Indexing m3u. Low memory consumption.
     * Faster speed for random access to each entry
     * Can be used with HUGE m3u files
     *
     * Returns array of groups each contains
     * array of file positions for each entries
     *
     * @return array[]
     */
    public function indexFile()
    {
        $data = array();
        if ($this->m3u_file === null) {
            hd_print("indexFile: Bad file");
            return $data;
        }

        $this->m3u_file->rewind();

        $t = microtime(1);
        $entry = new Entry();
        $pos = $this->m3u_file->ftell();
        while (!$this->m3u_file->eof()) {
            if (!$this->parseLine($this->m3u_file->fgets(), $entry)) continue;

            $group_name = $entry->getGroupTitle();
            if (!array_key_exists($group_name, $data)) {
                $data[$group_name] = array();
            }

            $data[$group_name][] = $pos;
            $entry = new Entry();
            $pos = $this->m3u_file->ftell();
        }

        hd_print("indexFile " . (microtime(1) - $t) . " secs");
        return $data;
    }

    /**
     * Load m3u into the memory for faster parsing
     * But may cause OutOfMemory for large files
     *
     * @return Entry[]
     */
    public function parseInMemory()
    {
        if (!file_exists($this->file_name)) {
            hd_print("parseInMemory: Can't read file: $this->file_name");
            return array();
        }

        $t = microtime(1);
        $lines = file($this->file_name, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        $data = array();
        $entry = new Entry();
        foreach($lines as $line) {
            if (!$this->parseLine($line, $entry)) continue;

            $data[] = $entry;
            $entry = new Entry();
        }

        hd_print("parseInMemory " . (microtime(1) - $t) . " secs");
        return $data;
    }

    /**
     * get entry by idx
     *
     * @param int $idx
     * @return Entry
     */
    public function getEntryByIdx($idx)
    {
        if ($this->m3u_file === null) {
            hd_print('getEntryByIdx: Bad file');
            return null;
        }
        $this->m3u_file->fseek((int)$idx);
        $entry = new Entry();
        while (!$this->m3u_file->eof()) {
            if ($this->parseLine($this->m3u_file->fgets(), $entry)) {
                return $entry;
            }
        }

        return null;
    }

    /**
     * get entry by idx
     *
     * @param int $idx
     * @return string
     */
    public function getTitleByIdx($idx)
    {
        if ($this->m3u_file === null) {
            hd_print('getTitleByIdx: Bad file');
            return null;
        }

        $this->m3u_file->fseek((int)$idx);
        while (!$this->m3u_file->eof()) {
            $line = trim($this->m3u_file->fgets());
            if (empty($line)) continue;

            if (!self::isTag($line)) break;

            if (self::isExtInf($line)) {
                $entry = new Entry();
                $entry->setExtInf(new ExtInf($line, false));
                return $entry->getTitle();
            }
        }

        return '';
    }

    ///////////////////////////////////////////////////////////

    /**
     * Parse one line
     *
     * @param string $line
     * @param Entry& $entry
     * @return bool
     */
    protected function parseLine($line, &$entry)
    {
        $line = trim($line);
        if (empty($line)) {
            return false;
        }

        if (self::isTag($line)) {
            if (self::isExtInf($line)) {
                $entry->setExtInf(new ExtInf($line));
            } else if (self::isExtGrp($line)) {
                $entry->setExtGrp(new ExtGrp($line));
            }
        } else {
            $entry->setPath($line);
            return true;
        }

        return false;
    }

    /**
     * @param string $line
     * @return bool
     */
    protected static function isTag($line)
    {
        return !empty($line) && $line[0] === '#';
    }

    /**
     * @param string $line
     * @return bool
     */
    protected static function isExtM3u($line)
    {
        return stripos($line, '#EXTM3U') === 0;
    }

    /**
     * @param string $line
     * @return bool
     */
    protected static function isExtInf($line)
    {
        return stripos($line, '#EXTINF:') === 0;
    }

    /**
     * @param string $line
     * @return bool
     */
    protected static function isExtGrp($line)
    {
        return stripos($line, '#EXTGRP:') === 0;
    }
}

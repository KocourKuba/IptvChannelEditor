<?php

require_once 'Entry.php';

class M3uParser
{
    private $file_name;
    private $vod_pattern;

    public function __construct($file_name, $vod_pattern = null)
    {
        $this->file_name = $file_name;
        if (!empty($vod_pattern))
            $this->vod_pattern = "/$vod_pattern/";
    }

    /**
     * Parse m3u by seeks file, slower but
     * less memory consumption for large m3u files
     *
     * @return Entry[]
     */
    public function parseInFile()
    {
        $m3u_file = new SplFileObject($this->file_name);
        $m3u_file->setFlags(SplFileObject::DROP_NEW_LINE);
        if (!$m3u_file->valid()) {
            hd_print("Can't read file: $this->file_name");
            return array();
        }

        $data = array();
        $entry = new Entry();
        while (!$m3u_file->eof()) {
            if (!$this->parseLine($m3u_file->fgets(), $entry)) continue;

            $data[] = $entry;
            $entry = new Entry();
        }

        return $data;
    }

    /**
     * Load m3u into the memory for faster parsing
     * But may cause OutOfMemory for huge files
     *
     * @return Entry[]
     */
    public function parseInMemory()
    {
        if (!file_exists($this->file_name)) {
            hd_print("Can't read file: $this->file_name");
            return array();
        }

        $lines = file($this->file_name, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

        $data = array();
        for ($i = 0, $total = count($lines); $i < $total; ++$i) {
            if (!self::isComment($lines[$i])) {
                $data[] = $this->parseArrayLine($i, $lines);
            }
        }

        return $data;
    }

    /**
     * Collect groups (categories)
     *
     * @return string[]
     */
    public function collectGroups()
    {
        $m3u_file = new SplFileObject($this->file_name);
        $m3u_file->setFlags(SplFileObject::DROP_NEW_LINE);
        if (!$m3u_file->valid()) {
            hd_print("Can't read file: $this->file_name");
            return array();
        }

        $t = microtime(1);

        $data = array();
        $entry = new Entry();
        while (!$m3u_file->eof()) {
            if (!$this->parseLine($m3u_file->fgets(), $entry)) continue;

            if (!in_array($entry->getGroupTitle(), $data)) {
                $data[] = $entry->getGroupTitle();
            }
            $entry = new Entry();
        }

        hd_print('collectGroups at ' . (microtime(1) - $t) . ' secs');
        return $data;
    }

    /**
     * search keyword in file and return matched entries
     *
     * @param string $keyword
     * @return Entry[]
     */
    public function searchEntry($keyword)
    {
        $m3u_file = new SplFileObject($this->file_name);
        $m3u_file->setFlags(SplFileObject::DROP_NEW_LINE);
        if (!$m3u_file->valid()) {
            hd_print("Can't read file.");
            return array();
        }

        $t = microtime(1);
        $data = array();
        $entry = new Entry();
        $entry_idx = 0;
        while (!$m3u_file->eof()) {
            if (!$this->parseLine($m3u_file->fgets(), $entry)) continue;

            $title = $entry->getTitle();
            $search_in = utf8_encode(mb_strtolower($title, 'UTF-8'));
            if (strpos($search_in, $keyword) !== false) {
                $data[$entry_idx] = $entry;
                $entry = new Entry();
            }
            $entry_idx++;
        }

        hd_print('searchEntry at ' . (microtime(1) - $t) . ' secs');
        return $data;
    }

    /**
     * collect entries matched $group_name
     *
     * @param string $group_name
     * @return Entry[]
     */
    public function collectEntriesByGroup($group_name)
    {
        $m3u_file = new SplFileObject($this->file_name);
        $m3u_file->setFlags(SplFileObject::DROP_NEW_LINE);
        if (!$m3u_file->valid()) {
            hd_print("Can't read file.");
            return array();
        }

        $t = microtime(1);

        $data = array();
        $entry = new Entry();
        $entry_idx = 0;
        while (!$m3u_file->eof()) {
            if (!$this->parseLine($m3u_file->fgets(), $entry)) continue;

            if ($group_name === $entry->getGroupTitle()) {
                $data[$entry_idx] = $entry;
            }

            $entry_idx++;
            $entry = new Entry();
        }

        hd_print('collectGroupEntries at ' . (microtime(1) - $t) . ' secs');
        hd_print("memory usage: " . memory_get_usage());
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
        $m3u_file = new SplFileObject($this->file_name);
        $m3u_file->setFlags(SplFileObject::DROP_NEW_LINE);
        if (!$m3u_file->valid()) {
            hd_print("Can't read file.");
            return null;
        }

        $t = microtime(1);
        $entry = new Entry();
        $entry_idx = 0;
        while (!$m3u_file->eof() && $entry_idx <= $idx) {
            if (!$this->parseLine($m3u_file->fgets(), $entry)) continue;

            if ($idx === $entry_idx++) {
                hd_print('getEntryByIdx at ' . (microtime(1) - $t) . ' secs');
                return $entry;
            }
            $entry = new Entry();
        }

        hd_print('getEntryByIdx fail to find at ' . (microtime(1) - $t) . ' secs');
        return null;
    }

    ///////////////////////////////////////////////////////////

    /**
     * Parse one line
     *
     * @param int& $i
     * @param string[] $lines
     * @return Entry
     */
    protected function parseArrayLine(&$i, $lines)
    {
        $entry = new Entry();

        for ($total = count($lines); $i < $total; $i++) {
            $nextLine = trim($lines[$i]);
            if (empty($nextLine) || self::isExtM3u($nextLine) || self::isComment($nextLine)) {
                continue;
            }

            if (self::isExtInf($nextLine)) {
                $entry->setExtInf(new ExtInf($nextLine));
                if (!empty($this->vod_pattern) && preg_match($this->vod_pattern, $entry->getTitle(), $match) && isset($match['title'])) {
                    $title = $match['title'];
                    $entry->setParsedTitle($title);
                }
            } else if (self::isExtGrp($nextLine)) {
                $entry->setExtGrp(new ExtGrp($nextLine));
            } else {
                $entry->setPath($nextLine);
                break;
            }
        }

        return $entry;
    }

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
        if (empty($line) || self::isExtM3u($line) || self::isComment($line)) {
            return false;
        }

        if (self::isExtInf($line)) {
            $entry->setExtInf(new ExtInf($line));
            if (!empty($this->vod_pattern) && preg_match($this->vod_pattern, $entry->getTitle(), $match) && isset($match['title'])) {
                $title = $match['title'];
                $entry->setParsedTitle($title);
            }
        } else if (self::isExtGrp($line)) {
            $entry->setExtGrp(new ExtGrp($line));
        } else {
            $entry->setPath($line);
            return true;
        }

        return false;
    }

    /**
     * @param string $str
     */
    protected function removeBom(&$str)
    {
        if (strpos($str, "\xEF\xBB\xBF") === 0) {
            $str = substr($str, 3);
        }
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

    /**
     * @param string $line
     * @return bool
     */
    protected static function isComment($line)
    {
        return $line[0] === '#' && !self::isExtInf($line) && !self::isExtGrp($line) && !self::isExtM3u($line);
    }

    /**
     * @param string $line
     * @return bool
     */
    protected static function isExtM3u($line)
    {
        return stripos($line, '#EXTM3U') === 0;
    }
}

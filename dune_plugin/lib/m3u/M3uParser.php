<?php

require_once 'Entry.php';

class M3uParser
{
    /**
     * Parse m3u file
     *
     * @param string $file
     * @return Entry[]
     * @throws Exception
     */
    public function parseFile($file)
    {
        $str = @file_get_contents($file);
        if (false === $str) {
            throw new Exception("Can't read file.");
        }

        return $this->parseStr($str);
    }

    /**
     * Parse m3u string
     *
     * @param string $str
     * @return Entry[]
     */
    public function parseStr($str)
    {
        $this->removeBom($str);

        $data = array();
        $lines = explode("\n", $str);

        for ($i = 0, $l = count($lines); $i < $l; ++$i) {
            $lineStr = trim($lines[$i]);
            if (empty($lineStr) || self::isComment($lineStr)) {
                continue;
            }

            $data[] = $this->parseLine($i, $lines);
        }

        return $data;
    }

    /**
     * Parse m3u string
     *
     * @param array $lines
     * @return Entry[]
     */
    public function parseArray($lines)
    {
        $data = array();
        for ($i = 0, $total = count($lines); $i < $total; ++$i) {
            $lineStr = trim($lines[$i]);
            if (self::isComment($lineStr)) {
                continue;
            }
            $data[] = $this->parseLine($i, $lines);
        }

        return $data;
    }

    /**
     * Parse one line
     *
     * @param int& $i
     * @param string[] $lines
     * @return Entry
     */
    protected function parseLine(&$i, $lines)
    {
        $entry = new Entry();

        for ($total = count($lines); $i < $total; $i++) {
            $nextLine = trim($lines[$i]);
            if (empty($nextLine) || self::isExtM3u($nextLine) || self::isComment($nextLine)) continue;

            if (self::isExtInf($nextLine)) {
                $entry->setExtInf(new ExtInf($nextLine));
                continue;
            }

            if (self::isExtGrp($nextLine)) {
                $entry->setExtGrp(new ExtGrp($nextLine));
                continue;
            }

            $entry->setPath($nextLine);
            break;
        }

        return $entry;
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

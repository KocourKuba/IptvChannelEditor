<?php

interface Archive
{
    /**
     * @return string
     */
    public function get_id();

    /**
     * @return array
     */
    public function get_archive_def();
}

<?php

require_once 'lib/tv/default_channel.php';

class StarnetChannel extends DefaultChannel
{
    public function __construct($id,
                                $channel_id,
                                $title,
                                $icon_url,
                                $streaming_url,
                                $has_archive,
                                $epg_id,
                                $tvg_id,
                                $is_protected,
                                $timeshift_hours)
    {
        parent::__construct(
            $id,
            $channel_id,
            $title,
            $icon_url,
            $streaming_url,
            $has_archive,
            0,
            $epg_id,
            $tvg_id,
            $is_protected,
            $timeshift_hours);
    }
}

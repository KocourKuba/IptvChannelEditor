<?php
require_once 'lib/default_config.php';

class iptvonline_config extends default_config
{
    /**
     * @param $channel Channel
     * @param $epg_source string
     * @return string
     */
    public function get_epg_id($channel, $epg_source)
    {
        $epg_id = parent::get_epg_id($channel, $epg_source);

        if (($epg_source !== Plugin_Constants::EPG_INTERNAL) && strpos($epg_id, 'X') === 0) {
            $epg_id = substr($epg_id, 1);
        }

        return $epg_id;
    }
}

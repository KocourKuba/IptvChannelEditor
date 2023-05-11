<?php
require_once 'lib/default_config.php';

class viplime_config extends default_config
{
    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     * @throws Exception
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $channel->set_ext_param(Ext_Params::M_QUALITY_ID, $this->get_quality_id($plugin_cookies));

        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }
}

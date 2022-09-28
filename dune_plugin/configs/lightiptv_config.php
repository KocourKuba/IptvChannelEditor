<?php
require_once 'lib/default_config.php';

class lightiptv_config extends default_config
{
    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param int $archive_ts
     * @param Channel $channel
     * @return string
     */
    public function GenerateStreamUrl($plugin_cookies, $archive_ts, Channel $channel)
    {
        $channel->set_ext_param(M_PASSWORD, $this->get_password($plugin_cookies));
        return parent::GenerateStreamUrl($plugin_cookies, $archive_ts, $channel);
    }
}

<?php
require_once 'cbilling_vod_impl.php';

class antifriz_config extends Cbilling_Vod_Impl
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_PIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://antifriz.tv/playlist/{PASSWORD}.m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+):(?<port>.+)/s/(?<token>.+)/(?<id>.+)/.*$');

        $this->set_stream_param(HLS,CU_TYPE, 'flussonic');
        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}:{PORT}/s/{TOKEN}/{ID}/video.m3u8');
        $this->set_stream_param(HLS,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.m3u8?token={TOKEN}');

        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/{ID}/mpegts?token={TOKEN}');
        $this->set_stream_param(MPEG,URL_ARC_TEMPLATE, 'http://{DOMAIN}/{ID}/{CU_SUBST}-{START}-{DURATION}.ts?token={TOKEN}');

        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_URL, self::API_HOST .'/epg/{ID}/?date=');
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        $account_data = parent::GetAccountInfo($plugin_cookies, $force);
        if ($account_data === false) {
            return false;
        }

        $this->account_data = $account_data;
        return $account_data;
    }
}

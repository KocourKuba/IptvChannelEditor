<?php
require_once 'lib/default_config.php';

class oneott_config extends default_config
{
    public function load_default()
    {
        $this->set_feature(ACCESS_TYPE, ACCOUNT_LOGIN);
        $this->set_feature(PLAYLIST_TEMPLATE, 'http://list.1ott.net/api/{TOKEN}/high/ottplay.m3u8');
        $this->set_feature(URI_PARSE_PATTERN, '^https?://(?<domain>.+)/~(?<token>.+)/(?<id>.+)/hls/pl\.m3u8$');

        $this->set_stream_param(HLS,URL_TEMPLATE, 'http://{DOMAIN}/~{TOKEN}/{ID}/hls/pl.m3u8');

        $this->set_stream_param(MPEG,CU_TYPE, 'shift');
        $this->set_stream_param(MPEG,URL_TEMPLATE, 'http://{DOMAIN}/~{TOKEN}/{ID}');

        $this->set_epg_param(EPG_FIRST,EPG_URL,'http://epg.propg.net/{ID}/epg2/{DATE}');
        $this->set_epg_param(EPG_FIRST,EPG_DATE_FORMAT, '{YEAR}-{MONTH}-{DAY}');
        $this->set_epg_param(EPG_FIRST,EPG_ROOT, '');
        $this->set_epg_param(EPG_FIRST,EPG_START, 'start');
        $this->set_epg_param(EPG_FIRST,EPG_END, 'stop');
        $this->set_epg_param(EPG_FIRST,EPG_NAME, 'epg');
        $this->set_epg_param(EPG_FIRST,EPG_DESC, 'desc');

        $this->set_epg_param(EPG_SECOND,EPG_URL,'http://epg.drm-play.ml/1ott/epg/{ID}.json');
    }

    /**
     * Get information from the account
     * @param &$plugin_cookies
     * @param bool $force default false, force downloading playlist even it already cached
     * @return bool | array[] information collected and status valid otherwise - false
     */
    public function GetAccountInfo(&$plugin_cookies, $force = false)
    {
        hd_print("Collect information from account");

        if ($force === false && !empty($plugin_cookies->token)) {
            return true;
        }

        $login = $this->get_login($plugin_cookies);
        $password = $this->get_password($plugin_cookies);

        if (empty($login) || empty($password)) {
            hd_print("Login or password not set");
        }

        try {
            $url = sprintf('http://list.1ott.net/PinApi/%s/%s', $login, $password);
            // provider returns token used to download playlist
            $account_data = HD::DownloadJson($url);
            if (isset($account_data['token'])) {
                $plugin_cookies->token = $account_data['token'];
                return $account_data;
            }
        } catch (Exception $ex) {
            hd_print("User token not loaded");
        }

        return false;
    }
}

<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'OTT_KEY';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
    public static $CHANNELS_LIST = 'edem_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/edem/epg/%s.json'; // epg_id

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function AdjustStreamUri($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();
        // hd_print("Stream url:  " . $url);

        if ((int)$archive_ts > 0) {
            $now_ts = time();
            $url .= "?utc=$archive_ts&lutc=$now_ts";
            // hd_print("Archive TS:  " . $archive_ts);
            // hd_print("Now       :  " . $now_ts);
        }

        return $url;
    }

    /**
     * Get information from the provider
     * @param $plugin_cookies
     * @param array &$account_data
     * @param bool $force - ignored
     * @return bool true if information collected and packages exists
     */
    public static function GetAccountInfo($plugin_cookies, &$account_data, $force = false)
    {
        hd_print("Collect information from account " . static::$PLUGIN_SHOW_NAME);

        return true;
    }

    /**
     * Collect information from m3u8 playlist
     * @param $plugin_cookies
     * @return array
     */
    public static function GetPlaylistStreamInfo($plugin_cookies)
    {
        return array();
    }

    /**
     * Update url by provider additional parameters
     * @param $channel_id
     * @param $plugin_cookies
     * @param $ext_params
     * @return string
     */
    public static function UpdateStreamUri($channel_id, $plugin_cookies, $ext_params)
    {
        $subdomain = empty($plugin_cookies->subdomain_local) ? $plugin_cookies->subdomain : $plugin_cookies->subdomain_local;
        $token = empty($plugin_cookies->ott_key_local) ? $plugin_cookies->ott_key : $plugin_cookies->ott_key_local;
        if (empty($subdomain) || empty($token)) {
            hd_print("subdomain or token not defined");
        }

        // substitute subdomain token and id parameters
        $url = str_replace(
            array('{SUBDOMAIN}', '{ID}', '{TOKEN}'),
            array($subdomain, $channel_id, $token),
            static::$MEDIA_URL_TEMPLATE_HLS);
        return static::make_ts($url);
    }
}

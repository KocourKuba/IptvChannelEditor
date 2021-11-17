<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    // setup variables
    public static $ACCOUNT_TYPE = 'OTT_KEY';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/edem/epg/%s.json'; // epg_id

    // Views variables
    protected static $TV_CHANNEL_ICON_WIDTH = 84;
    protected static $TV_CHANNEL_ICON_HEIGHT = 48;

    /**
     * Transform url based on settings or archive playback
     * @param $plugin_cookies
     * @param $archive_ts
     * @param IChannel $channel
     * @return string
     */
    public static function TransformStreamUrl($plugin_cookies, $archive_ts, IChannel $channel)
    {
        $url = $channel->get_streaming_url();
        // hd_print("Stream url:  " . $url);

        $subdomain = empty($plugin_cookies->subdomain_local) ? $plugin_cookies->subdomain : $plugin_cookies->subdomain_local;
        $token = empty($plugin_cookies->ott_key_local) ? $plugin_cookies->ott_key : $plugin_cookies->ott_key_local;
        if (empty($subdomain) || empty($token)) {
            hd_print("TransformStreamUrl: parameters for {$channel->get_channel_id()} not defined!");
        } else {
            // substitute subdomain token parameters
            $url = str_replace(
                array('{SUBDOMAIN}', '{TOKEN}'),
                array($subdomain, $token),
                $url);
        }

        return self::UpdateArchiveUrlParams($url, $archive_ts);
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
}

<?php
require_once 'default_config.php';

class EdemPluginConfig extends DefaultConfig
{
    // info
    public static $PLUGIN_NAME = 'iEdem/iLook TV';
    public static $PLUGIN_SHORT_NAME = 'edem';
    public static $PLUGIN_VERSION = '2.7.1';
    public static $PLUGIN_DATE = '12.09.2021';

    // setup variables
    public static $ACCOUNT_TYPE = 'OTT_KEY';

    // tv
    public static $MEDIA_URL_TEMPLATE_HLS = 'http://ts://{SUBDOMAIN}/iptv/{TOKEN}/{ID}/index.m3u8';
    public static $CHANNELS_LIST = 'edem_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://epg.ott-play.com/edem/epg/%s.json'; // epg_id
    protected static $EPG2_URL_TEMPLATE = 'http://www.teleguide.info/kanal%s_%s.html'; // epg_id date(YYYYMMDD)
    protected static $EPG2_PARSER = 'html';
}

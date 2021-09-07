<?php
require_once 'default_config.php';

class SharavozPluginConfig extends DefaultConfig
{
    public static $PLUGIN_NAME = 'Sharavoz TV';
    public static $PLUGIN_SHORT_NAME = 'sharavoz';
    public static $PLUGIN_VERSION = '1.0.0';
    public static $PLUGIN_DATE = '07.09.2021';

    public static $MPEG_TS_SUPPORTED = true;
    public static $MEDIA_URL_TEMPLATE = 'http://ts://{SUBDOMAIN}/{ID}/index.m3u8?token={TOKEN}';
    public static $MEDIA_URL_TEMPLATE_TS = 'http://ts://{SUBDOMAIN}/{ID}/mpegts?token={TOKEN}';
    public static $CHANNELS_LIST = 'sharavoz_channel_list.xml';
    protected static $EPG1_URL_TEMPLATE = 'http://api.program.spr24.net/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)
    protected static $EPG2_URL_TEMPLATE = 'http://epg.arlekino.tv/api/program?epg=%s&date=%s'; // epg_id date(YYYYMMDD)

}

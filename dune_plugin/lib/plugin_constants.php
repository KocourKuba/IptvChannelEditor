<?php

require_once 'stream_params.php';
require_once 'ext_params.php';

class Plugin_Constants
{
    const ACCOUNT_UNKNOWN = 'unknown';
    const ACCOUNT_OTT_KEY = 'ottkey';
    const ACCOUNT_PIN     = 'pin';
    const ACCOUNT_LOGIN   = 'login';

    const HLS  = 'hls';
    const MPEG = 'mpeg';

    // Non configurable parameters by external config
    const /* (bool) */ TV_FAVORITES_SUPPORTED = 'tv_fav';
    const /* (bool) */ VOD_QUALITY_SUPPORTED = 'vod_quality';
    const /* (bool) */ VOD_AUDIO_SUPPORTED = 'vod_audio';
    const /* (bool) */ VOD_FILTER_SUPPORTED = 'vod_filter';
    const /* (bool) */ BALANCE_SUPPORTED = 'balance_support';
    const /* (bool) */ VOD_FILTERS = 'vod_filters';

    // parameters loaded from external config

    // common
    const /* (char *) */ CGI_BIN = 'cgi_bin';
    const /* (char *) */ CHANNEL_ID = 'id';
    const /* (char *) */ ACCESS_TYPE = 'access_type';
    const /* (bool)   */ PROVIDER_API_URL = 'provider_api_url';
    const /* (char*)  */ USER_AGENT = 'user_agent';
    const /* (char*)  */ DEV_CODE = 'dev_code';
    const /* (array ) */ PLAYLIST_TEMPLATES = 'playlist_templates';
    const /* (int )   */ PLAYLIST_TEMPLATE_INDEX = 'playlist_template_index';

    const /* (char *) */ VOD_ENGINE = 'vod_engine';
    const /* (array)  */ VOD_TEMPLATES = 'vod_templates';
    const /* (array)  */ XMLTV_SOURCES = 'xmltv_sources';

    const /* (char*)  */ PLAYLIST_NAME = 'name';
    const /* (char *) */ PL_TEMPLATE = 'pl_template';
    const /* (char *) */ PARSE_REGEX = 'parse_regex';
    const /* (char *) */ PARSE_REGEX_TITLE = 'parse_regex_title';
    const /* (char *) */ TAG_ID_MATCH = 'tag_id_match';
    const /* (char*)  */ URL_PREFIX = 'url_prefix';
    const /* (char*)  */ URL_PARAMS = 'url_params';

    // streams parameters
    const /* array */ STREAMS_CONFIG = 'streams_config';

    // epg parameters
    const /* array */ EPG_CUSTOM_SOURCE = 'custom_epg_urls';
    const /* array */ EPG_PARAMS = 'epg_params';

    // server parameters
    const /* array */ SERVERS_LIST = 'servers_list';

    // device parameters
    const /* array */ DEVICES_LIST = 'devices_list';

    // quality parameters
    const /* array */ QUALITIES_LIST = 'qualities_list';

    // profile parameters
    const /* array */ PROFILES_LIST = 'profiles_list';

    // domains parameters
    const /* array */ DOMAINS_LIST = 'domains_list';

    const LIST_ID = 'id';
    const LIST_NAME = 'name';

    // not used in config
    const EPG_FIRST = 'first';
    const EPG_SECOND = 'second';
    const EPG_INTERNAL = 'internal';
}

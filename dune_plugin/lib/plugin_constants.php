<?php

require_once 'stream_params.php';
require_once 'epg_params.php';
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
    const /* (char) */ API_REQUEST_URL = 'api_request_url';
    const /* (bool) */ TV_FAVORITES_SUPPORTED = 'tv_fav';
    const /* (bool) */ VOD_QUALITY_SUPPORTED = 'vod_quality';
    const /* (bool) */ VOD_FILTER_SUPPORTED = 'vod_filter';
    const /* (bool) */ BALANCE_SUPPORTED = 'balance_support';

    // parameters loaded from external config

    // common
    const /* (char *) */ CGI_BIN = 'cgi_bin';
    const /* (char *) */ CHANNEL_ID = 'id';
    const /* (char *) */ ACCESS_TYPE = 'access_type';
    const /* (char *) */ SHORT_NAME = 'short_name';
    const /* (bool)   */ SQUARE_ICONS = 'square_icons';
    const /* (char*)  */ USER_AGENT = 'user_agent';
    const /* (array ) */ PLAYLIST_TEMPLATES = 'playlist_templates';
    const /* (int )   */ PLAYLIST_TEMPLATE_INDEX = 'playlist_template_index';

    const /* (char *) */ VOD_SUPPORTED = 'vod_support';
    const /* (bool)   */ VOD_M3U = 'vod_m3u';
    const /* (array)  */ VOD_TEMPLATES = 'vod_templates';
    const /* (int)    */ VOD_TEMPLATES_INDEX = 'vod_template_index';

    const /* (char*)  */ PLAYLIST_NAME = 'name';
    const /* (char *) */ URI_TEMPLATE = 'pl_template';
    const /* (char *) */ PARSE_REGEX = 'parse_regex';
    const /* (char *) */ TAG_ID_MATCH = 'tag_id_match';

    // streams parameters
    const /* array */ STREAMS_CONFIG = 'streams_config';

    // epg parameters
    const /* array */ EPG_PARAMS = 'epg_params';

    // server parameters
    const /* array */ SERVERS_LIST = 'servers_list';

    // device parameters
    const /* array */ DEVICES_LIST = 'devices_list';

    // quality parameters
    const /* array */ QUALITIES_LIST = 'qualities_list';

    // profile parameters
    const /* array */ PROFILES_LIST = 'profiles_list';

    const LIST_ID = 'id';
    const LIST_NAME = 'name';

    // not used in config
    const EPG_FIRST = 'first';
    const EPG_SECOND = 'second';
    const EPG_PARSER = 'epg_parser';
    const EPG_ID_MAPPER = 'epg_id_mapper';
}
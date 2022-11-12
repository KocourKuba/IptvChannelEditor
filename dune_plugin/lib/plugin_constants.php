<?php

require_once 'stream_params.php';
require_once 'epg_params.php';
require_once 'ext_params.php';

const HLS  = 'hls';
const MPEG = 'mpeg';

class Plugin_Constants
{
    const ACCOUNT_UNKNOWN = 'unknown';
    const ACCOUNT_OTT_KEY = 'ottkey';
    const ACCOUNT_PIN     = 'pin';
    const ACCOUNT_LOGIN   = 'login';

    // Non configurable parameters by external config
    const API_REQUEST_URL = 'api_request_url';
    const TV_FAVORITES_SUPPORTED = 'tv_fav';
    const VOD_SUPPORTED = 'vod_support';
    const VOD_M3U = 'vod_m3u';
    const VOD_QUALITY_SUPPORTED = 'vod_quality';
    const VOD_FILTER_SUPPORTED = 'vod_filter';
    const VOD_PLAYLIST_URL = 'provider_vod_url';
    const VOD_PARSE_PATTERN = 'vod_parse_pattern';
    const BALANCE_SUPPORTED = 'balance_support';
    const SQUARE_ICONS = 'square_icons';

    // parameters loaded from external config

    // common
    const /* (char *) */ CHANNEL_ID = 'id';
    const /* (char *) */ ACCESS_TYPE = 'access_type';
    const /* (char *) */ SHORT_NAME = 'short_name';
    const /* (char *) */ PLAYLIST_TEMPLATE = 'playlist_template';
    const /* (char *) */ PLAYLIST_TEMPLATE2 = 'playlist_template2';
    const /* (char *) */ URI_PARSE_PATTERN = 'uri_parse_pattern';
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
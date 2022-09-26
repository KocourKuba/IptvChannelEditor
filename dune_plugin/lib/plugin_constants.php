<?php

const ACCOUNT_OTT_KEY = 0;
const ACCOUNT_PIN     = 1;
const ACCOUNT_LOGIN   = 2;

const HLS  = 'hls';
const MPEG = 'mpeg';

// Non configurable parameters by external config
const TV_FAVORITES_SUPPORTED   = 'tv_fav';
const VOD_SUPPORTED            = 'vod_support';
const VOD_FAVORITES_SUPPORTED  = 'vod_fav';
const VOD_QUALITY_SUPPORTED    = 'vod_quality';
const VOD_FILTER_SUPPORTED     = 'vod_filter';
const VOD_LAZY_LOAD            = 'vod_lazy';
const VOD_PARSE_PATTERN        = 'vod_parse_pattern';
const VOD_PLAYLIST_URL         = 'vod_playlist_url';
const BALANCE_SUPPORTED        = 'balance_support';
const DEVICE_OPTIONS           = 'device_options';
const SERVER_OPTIONS           = 'server_support';
const QUALITY_OPTIONS          = 'quality_support';
const SQUARE_ICONS             = 'square_icons';

// ext parameters keys
const M_SUBDOMAIN = 'subdomain';
const M_DOMAIN    = 'domain';
const M_PORT      = 'port';
const M_ID        = 'id';
const M_TOKEN     = 'token';
const M_LOGIN     = 'login';
const M_PASSWORD  = 'password';
const M_INT_ID    = 'int_id';
const M_HOST      = 'host';
const M_QUALITY   = 'quality';

// parameters loaded from external config

// common
const ACCESS_TYPE          = 'access_type';
const USE_TOKEN_AS_ID      = 'use_token_as_id';
const PLAYLIST_TEMPLATE    = 'playlist_template';
const PLAYLIST_TEMPLATE2   = 'playlist_template2';
const URI_ID_PARSE_PATTERN = 'uri_id_parse_pattern';
const URI_PARSE_PATTERN    = 'uri_parse_pattern';

// streams parameters
const STREAMS_CONFIG   = 'streams_config';
const STREAM_TYPE      = 'stream_type';
const URL_TEMPLATE     = 'uri_template';
const URL_ARC_TEMPLATE = 'uri_arc_template';
const CU_TYPE          = 'cu_type';
const CU_SUBST         = 'cu_subst';
const CU_DURATION      = 'cu_duration';

// epg parameters
const EPG_PARAMS       = 'epg_params';

// server parameters
const SERVERS_LIST     = 'servers_list';

// device parameters
const DEVICES_LIST     = 'device_list';

// quality parameters
const QUALITIES_LIST   = 'qualities_list';

// profile parameters
const PROFILES_LIST    = 'profiles_list';

const LIST_ID          = 'id';
const LIST_NAME        = 'name';

// epg parameters keys
const EPG_URL          = 'epg_url';
const EPG_ROOT         = 'epg_root';
const EPG_START        = 'epg_start';
const EPG_END          = 'epg_end';
const EPG_NAME         = 'epg_name';
const EPG_DESC         = 'epg_desc';
const EPG_DATE_FORMAT  = 'epg_date_format';
const EPG_TIME_FORMAT  = 'epg_time_format';
const EPG_USE_DURATION = 'epg_use_duration';
const EPG_TIMEZONE     = 'epg_timezone';

// not used in config
const EPG_FIRST        = 'first';
const EPG_SECOND       = 'second';
const EPG_PARSER       = 'epg_parser';
const EPG_USE_MAPPER   = 'epg_use_mapper';
const EPG_MAPPER_URL   = 'epg_mapper_url';
const EPG_ID_MAPPER    = 'epg_id_mapper';

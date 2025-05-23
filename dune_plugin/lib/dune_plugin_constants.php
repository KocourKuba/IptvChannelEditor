<?php

# Common actions
const ACTION_ADD_FAV = 'add_favorite';
const ACTION_CHANGE_PLAYLIST = 'change_playlist';
const ACTION_EXTERNAL_PLAYER = 'use_external_player';
const ACTION_INTERNAL_PLAYER = 'use_internal_player';
const ACTION_FOLDER_SELECTED = 'folder_selected';
const ACTION_PLAYLIST_SELECTED = 'playlist_selected';
const ACTION_FILE_SELECTED = 'file_selected';
const ACTION_ITEM_ADD = 'item_add';
const ACTION_ITEM_DELETE = 'item_delete';
const ACTION_ITEM_DOWN = 'item_down';
const ACTION_ITEM_REMOVE = 'item_remove';
const ACTION_ITEM_UP = 'item_up';
const ACTION_ITEMS_CLEAR = 'items_clear';
const ACTION_ITEMS_EDIT = 'items_edit';
const ACTION_ITEMS_SORT = 'items_sort';
const ACTION_RESET_ITEMS_SORT = 'reset_items_sort';
const ACTION_OPEN_FOLDER = 'open_folder';
const ACTION_PLAY_FOLDER = 'play_folder';
const ACTION_PLAY_ITEM = 'play_item';
const ACTION_REFRESH_SCREEN = 'refresh_screen';
const ACTION_TOGGLE_ICONS_TYPE = 'toggle_icons_type';
const ACTION_RELOAD = 'reload';
const ACTION_RESET_DEFAULT = 'reset_default';
const ACTION_SETTINGS = 'settings';
const ACTION_ZOOM_POPUP_MENU = 'zoom_popup_menu';
const ACTION_ZOOM_APPLY = 'zoom_apply';
const ACTION_ZOOM_SELECT = 'zoom_select';
const ACTION_EMPTY = 'empty';
const ACTION_PLUGIN_INFO = 'plugin_info';
const ACTION_CHANGE_GROUP_ICON = 'change_group_icon';
const ACTION_CHANGE_BACKGROUND = 'change_background';
const ACTION_CHANNEL_INFO = 'channel_info';
const ACTION_CHANGE_EPG_SOURCE = 'change_epg_source';
const ACTION_FILTER = 'action_filter';
const ACTION_CREATE_FILTER = 'create_filter';
const ACTION_RUN_FILTER = 'run_filter';
const ACTION_CREATE_SEARCH = 'create_search';
const ACTION_NEW_SEARCH = 'new_search';
const ACTION_RUN_SEARCH = 'run_search';
const ACTION_SEARCH = 'action_search';
const ACTION_WATCHED = 'watched';
const ACTION_QUALITY = 'quality';
const ACTION_AUDIO = 'audio';
const ACTION_CHANNELS_SETTINGS = 'channels_settings';
const ACTION_NEED_CONFIGURE = 'configure';
const ACTION_BALANCE = 'balance';
const ACTION_INFO = 'info';

# Special groups ID
const FAVORITES_GROUP_ID = '##favorites##';
const ALL_CHANNEL_GROUP_ID = '##all_channels##';
const HISTORY_GROUP_ID = '##playback_history_tv_group##';
const VOD_GROUP_ID = '##mediateka##';

# Common parameters
const PARAM_ADULT_PASSWORD = 'adult_password';
const PARAM_PLAYLIST = 'playlist';
const PARAM_PLAYLIST_IDX = 'playlist_idx';
const PARAM_VOD_IDX = 'vod_idx';
const PARAM_FAVORITES = 'favorites';
const PARAM_ASK_EXIT = 'ask_exit';
const PARAM_SHOW_ALL = 'show_all';
const PARAM_SHOW_FAVORITES = 'show_favorites';
const PARAM_SHOW_HISTORY = 'show_history';
const PARAM_SHOW_VOD = 'show_vod';
const PARAM_VOD_LAST = 'vod_last';
const PARAM_SQUARE_ICONS = 'square_icons';
const PARAM_ICONS_IN_ROW = 'icons_in_row';
const PARAM_SHOW_CHANNEL_CAPTION = 'show_channel_caption';
const PARAM_HISTORY_PATH = 'history_path';
const PARAM_CHANNELS_LIST_PATH = 'channels_list_path';
const PARAM_CHANNELS_LIST_NAME = 'channels_list_name';
const PARAM_CHANNELS_SOURCE = 'channels_source';
const PARAM_CHANNELS_URL = 'channels_url';
const PARAM_CHANNELS_DIRECT_URL = 'channels_direct_url';
const PARAM_EPG_CACHE_ENGINE = 'epg_cache_engine';
const PARAM_EPG_CACHE_PARAMETERS = 'epg_cache_parameters';
const PARAM_EPG_CACHE_TTL = 'epg_cache_ttl';
const PARAM_EPG_SHIFT = 'epg_shift';
const PARAM_EPG_FONT_SIZE = 'epg_font_size';
const PARAM_CACHE_PATH = 'xmltv_cache_path';
const PARAM_XMLTV_CUR_SOURCE_KEY = 'cur_xmltv_key';
const PARAM_XMLTV_SOURCE = 'xmltv_source';
const PARAM_DUNE_PARAMS = 'dune_params';
const PARAM_CHANNELS_ZOOM = 'channels_zoom';
const PARAM_CHANNEL_PLAYER = 'channel_player';
const PARAM_USER_CATCHUP = 'user_catchup';
const PARAM_TV_HISTORY_ITEMS = '_tv_history_items';
const PARAM_BUFFERING_TIME = 'buffering_time';
const PARAM_ARCHIVE_DELAY_TIME = 'archive_delay_time';
const PARAM_ENABLE_DEBUG = 'enable_debug';
const PARAM_PER_CHANNELS_ZOOM = 'per_channels_zoom';
const PARAM_SHOW_EXT_EPG = 'show_ext_epg';
const PARAM_FAKE_EPG = 'fake_epg';
const PARAM_USE_UPDATER_PROXY = 'use_proxy';
const PARAM_STREAM_FORMAT = 'stream_format';
const PARAM_VOD_DEFAULT_QUALITY = 'variant';
const PARAM_LOGIN = 'login';
const PARAM_PASSWORD = 'password';
const PARAM_CHANNEL_POSITION = 'channel_position';

const EPG_SOURCES_SEPARATOR_TAG = 'special_source_separator_tag';
const ENGINE_JSON = 'json';
const ENGINE_XMLTV = 'xmltv';
const XMLTV_CACHE_AUTO = 'auto';
const XMLTV_CACHE_MANUAL = 'manual';
const EPG_CACHE_SUBDIR = 'epg_cache';
const HISTORY_SUBDIR = 'history';
const EPG_FAKE_EPG = 2;

# HTTP params
const USER_AGENT = 'User-Agent';
const REFERER = 'Referer';

# Media types patterns
const AUDIO_PATTERN = 'mp3|ac3|wma|ogg|ogm|m4a|aif|iff|mid|mpa|ra|wav|flac|ape|vorbis|aac|a52';
const VIDEO_PATTERN = 'avi|mp4|mpg|mpeg|divx|m4v|3gp|asf|wmv|mkv|mov|ogv|vob|flv|ts|3g2|swf|ps|qt|m2ts';
const IMAGE_PREVIEW_PATTERN = 'png|jpg|jpeg|bmp|gif|aai';
const IMAGE_PATTERN = '|psd|pspimage|thm|tif|yuf|svg|ico|djpg|dbmp|dpng';
const PLAYLIST_PATTERN = 'm3u|m3u8';
const EPG_PATTERN = 'xml|xmltv|gz';
const HTTP_PATTERN = '|^(https?)://|';
const TS_REPL_PATTERN = '/^(https?:\/\/)(.+)$/';

# Mounted storages path
const DUNE_MOUNTED_STORAGES_PATH = '/tmp/mnt/storage';
const DUNE_APK_STORAGE_PATH = '/sdcard/DuneHD/Dune_backup';

const JSON_ENCODE_STANDARD = 0;
const JSON_ENCODE_RAW = 1;

if (!defined('JSON_UNESCAPED_SLASHES'))
    define("JSON_UNESCAPED_SLASHES", 64);
if (!defined('JSON_PRETTY_PRINT'))
    define('JSON_PRETTY_PRINT', 128);
if (!defined('JSON_UNESCAPED_UNICODE'))
    define('JSON_UNESCAPED_UNICODE', 256);

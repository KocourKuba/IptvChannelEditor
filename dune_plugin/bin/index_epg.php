<?php
require_once "cgi_config.php";

set_include_path(get_include_path(). PATH_SEPARATOR . DuneSystem::$properties['install_dir_path']);

global $LOG_FILE;

require_once 'lib/ordered_array.php';
require_once 'lib/hashed_array.php';
require_once 'lib/hd.php';
require_once 'lib/epg_manager_sql.php';

hd_print("Script start: index_epg");
hd_print("Version: " . DuneSystem::$properties['plugin_version']);
hd_print("Log: $LOG_FILE");

$parameters = HD::get_data_items('common.settings', true, false);
$debug = isset($parameters[PARAM_ENABLE_DEBUG]) ? $parameters[PARAM_ENABLE_DEBUG] : SetupControlSwitchDefs::switch_off;
set_debug_log($debug === SetupControlSwitchDefs::switch_on);

$cache_dir = isset($parameters[PARAM_CACHE_PATH]) ? $parameters[PARAM_CACHE_PATH] : get_data_path(EPG_CACHE_SUBDIR);
$cache_engine = isset($parameters[PARAM_EPG_CACHE_ENGINE]) ? $parameters[PARAM_EPG_CACHE_ENGINE] : ENGINE_JSON;
$cache_ttl = isset($parameters[PARAM_EPG_CACHE_TTL]) ? $parameters[PARAM_EPG_CACHE_TTL] : 3;
$xmltv_url = isset($parameters[PARAM_CUR_XMLTV_SOURCE]) ? $parameters[PARAM_CUR_XMLTV_SOURCE] : '';

if ($cache_engine === ENGINE_JSON) {
    hd_print("Engine set to JSON. Exiting");
    return;
}

if (class_exists('SQLite3')) {
    hd_debug_print("Using sqlite cache engine");
    $epg_man = new Epg_Manager_Sql(DuneSystem::$properties['plugin_version'], $cache_dir, $xmltv_url);
} else {
    hd_debug_print("Using legacy cache engine");
    $epg_man = new Epg_Manager(DuneSystem::$properties['plugin_version'], $cache_dir, $xmltv_url);
}

$epg_man->set_cache_ttl($cache_ttl);

$start = microtime(true);
$res = $epg_man->is_xmltv_cache_valid();
if ($res === -1) {
    hd_debug_print("Error load xmltv");
    return;
}

if ($res === 0) {
    $res = $epg_man->download_xmltv_source();
    if ($res === -1) {
        hd_debug_print("XMLTV source not downloaded, nothing to parse");
        return;
    }

    if ($res === 0) {
        hd_debug_print("XMLTV source is indexing now");
        return;
    }
}

$epg_man->index_xmltv_channels();
$epg_man->index_xmltv_positions();

hd_print("Script execution time: ". format_duration(round(1000 * (microtime(true) - $start))));

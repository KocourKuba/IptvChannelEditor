<?php
require_once($_SERVER["DOCUMENT_ROOT"] . "/shared_scripts/iptv_utils.php");

$url_params = parse_url(getenv("REQUEST_URI"));
if (isset($url_params['query'])) {
    parse_str($url_params['query'], $params);
}

if (isset($params['ver'])) {
    $ver = explode('.', $params['ver']);

    $logbuf = "========================================\n";
    $logbuf .= "date      : " . date("m.d.Y H:i:s") . "\n";
    $logbuf .= "url       : " . getenv("REQUEST_URI") . "\n";
    $logbuf .= "ip        : " . get_ip() . "\n";
    $logbuf .= "ver       : " . $params['ver'] . "\n";

    write_to_log($logbuf, 'editor.log');

    $name = "defaults_$ver[0].$ver[1].json";
    if (!file_exists($name)) {
        $name = '';
    }
}

if (!empty($name)) {
    header("HTTP/1.1 200 OK");
    header("Content-Type: application/json; charset=utf-8");
    header("Content-Length: " . filesize($name));
    echo file_get_contents($name);
} else {
    header("HTTP/1.1 404 Not found");
}

#!/bin/sh

plugin_root=$(builtin cd "../..";pwd)
plugin_name=$(basename $plugin_root)
plugin_xml="$plugin_root/dune_plugin.xml"

CURL=""
use_proxy=true
source /tmp/run/versions.txt
if [ $platform_kind == android ]; then
use_proxy=false
CURL="$FS_PREFIX/firmware/bin/curl"
elif (echo $platform_kind | grep -E -q "864."); then
CURL="$plugin_root/bin/curl.864x"
elif (echo $platform_kind | grep -E -q "865."); then
CURL="$plugin_root/bin/curl.865x"
elif (echo $platform_kind | grep -E -q "867."); then
CURL="$plugin_root/bin/curl.867x"
elif (echo $platform_kind | grep -E -q "87.."); then
use_proxy=false
CURL="$plugin_root/bin/curl.87xx"
fi

#log_path="/D/dune_plugin_logs"
user_agent="DuneHD/1.0 (product_id: $product; firmware_version: $firmware_version)"

if [[ $QUERY_STRING =~ ^https?: ]]; then
  $CURL -k -s -D - --location $QUERY_STRING --user-agent $user_agent
else
  update_url=$(cat "$plugin_xml"|sed -nr 's|^.*<real_url>(.*)</real_url>|\1|p')
  if [[ $update_url =~ ^https: && $use_proxy == true ]];
  then
    proxy_url=$(cat "$plugin_xml"|sed -nr 's|^.*<url>(.*)</url>|\1|p')
 	  echo -e "HTTP/1.0 200 OK"
 	  echo -e "Content-Type: text/xml\n"
    $CURL -k -s --location "$update_url?$QUERY_STRING" --user-agent $user_agent|sed -r "s|<url>(.*)</url>|<url>$proxy_url\?\1</url>|"
  else
    $CURL -k -s -D - --location "$update_url?$QUERY_STRING" --user-agent $user_agent
  fi
fi

exit;

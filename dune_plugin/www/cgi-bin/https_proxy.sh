#!/bin/sh
# called from PLUGIN_DIR/www/cgi-bin
thisdir=`dirname $0`
plugin_root=`builtin cd "$thisdir/../.." && pwd`
plugin_name=$(basename $plugin_root)
user_agent="DuneHD/1.0 (product_id: $product; firmware_version: $firmware_version)"
if [ -z "$HD_HTTP_LOCAL_PORT" ]; then
  HD_HTTP_LOCAL_PORT = "80";
fi
script_path="http://127.0.0.1:$HD_HTTP_LOCAL_PORT/cgi-bin/plugins/$plugin_name/https_proxy.sh"
log_path="$FS_PREFIX/tmp/run/$plugin_name-updater.log"

source $FS_PREFIX/tmp/run/versions.txt

CURL=""
if [ $platform_kind == android ]; then
CURL="$FS_PREFIX/firmware/bin/curl"
elif (echo $platform_kind | grep -E -q "864."); then
CURL="$plugin_root/bin/curl.864x"
elif (echo $platform_kind | grep -E -q "865."); then
CURL="$plugin_root/bin/curl.865x"
elif (echo $platform_kind | grep -E -q "867."); then
CURL="$plugin_root/bin/curl.867x"
elif (echo $platform_kind | grep -E -q "87.."); then
CURL="$plugin_root/bin/curl.87xx"
fi

echo "used curl: $CURL" > $log_path
echo "query: $QUERY_STRING" >> $log_path

URL=$(echo $QUERY_STRING|sed -r "s/(.+\.xml|\.gz|\.zip)\?.*$/\1/")
if $(echo $QUERY_STRING | grep -q ".dropbox"); then
  FIXED_URL=$(echo $QUERY_STRING|sed -r "s/(.*)\?(dune_auth.*)$/\1/")
else
  FIXED_URL=$QUERY_STRING
fi

if $(echo $URL | grep -q -E "\.xml$"); then
  echo "download update $FIXED_URL" >> $log_path
  echo -e "HTTP/1.0 200 OK" | tee -a $log_path
  echo -e "Content-Type: text/xml\n" | tee -a $log_path
  $CURL -k -s --location $FIXED_URL --user-agent $user_agent|sed -r "s|<url>(.*)</url>|<url>$script_path\?\1</url>|" | tee -a $log_path
else
  echo "download file $FIXED_URL">> $log_path
  $CURL -k -s -D - --location $FIXED_URL --user-agent $user_agent
fi

exit;

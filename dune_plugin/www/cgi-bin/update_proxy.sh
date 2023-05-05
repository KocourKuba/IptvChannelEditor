#!/bin/sh
CURL=""
source /tmp/run/versions.txt
if [ $platform_kind == android ]; then
CURL="$FS_PREFIX/firmware/bin/curl"
elif (echo $platform_kind | grep -E -q "864."); then
CURL="../../bin/curl.864x"
elif (echo $platform_kind | grep -E -q "865."); then
CURL="../../bin/curl.865x"
elif (echo $platform_kind | grep -E -q "867."); then
CURL="../../bin/curl.867x"
elif (echo $platform_kind | grep -E -q "87.."); then
CURL="../../bin/curl.87xx"
fi

$CURL -k -s -D - --location $QUERY_STRING --user-agent "DuneHD/1.0 (product_id: $product; firmware_version: $firmware_version)"
exit;

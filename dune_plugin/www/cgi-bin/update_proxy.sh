#!/bin/sh
CURL=""
source /tmp/run/versions.txt
if [ $platform_kind == android ]; then
CURL="/firmware/bin/curl"
elif (echo $platform_kind | grep -E -q "87.."); then
CURL="../../bin/curl.87xx"
elif (echo $platform_kind | grep -E -q "864."); then
CURL="../../bin/curl.864x"
elif (echo $platform_kind | grep -E -q "867."); then
CURL="../../bin/curl.867x"
elif (echo $platform_kind | grep -E -q "865."); then
CURL="../../bin/curl.865x"
fi

$CURL -k -s -D - --location $QUERY_STRING --user-agent "DuneHD/1.0"
exit;

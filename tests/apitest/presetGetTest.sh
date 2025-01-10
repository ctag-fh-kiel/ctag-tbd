#!/usr/bin/env bash

# repeat 10 times a get request on http://ctag-tbd.local/api/v1/getPresets/0

for i in {1..100}
do
  # send a few requests to the server http://ctag-tbd.local/api/v1/setPluginParam/0?id=rs_f0&current=xxxx with xxxx in between 0 and 4095
  for j in {1..10}
  do
    # generate a random number between 0 and 4095
    current=$((RANDOM % 4095))
    echo "current: $current"
    curl --keepalive -X GET "http://ctag-tbd.local/api/v1/setPluginParam/0?id=rs_f0&current=$current"
  done
  curl -X GET http://ctag-tbd.local/api/v1/getPluginParams/0
  sleep 5
  curl -X GET http://ctag-tbd.local/api/v1/getPresets/0
  sleep 5
done
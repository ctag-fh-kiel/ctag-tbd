#!/usr/bin/env bash

# endless loop
while true
do
    echo Setting Dings
    curl -X GET "http://ctag-tbd-bba.local/api/v1/setActivePlugin/0?id=TBDings" -H "accept: application/json"
    sleep 0.3
    echo Setting Void
    curl -X GET "http://ctag-tbd-bba.local/api/v1/setActivePlugin/0?id=Void" -H "accept: application/json"
    sleep 0.3
done
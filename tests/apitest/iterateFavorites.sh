#!/usr/bin/env bash

# endless loop
while true
do
    # for loop incremente variable i from 0 to 9
    for i in {0..9}
    do
        echo Favorite $i
        # call the api with POST with the current value of i http://ctag-tbd-bba.local/api/v1/favorites/recall/0
        curl -X POST "http://ctag-tbd-bba.local/api/v1/favorites/recall/$i" -H "accept: application/json" -H "Content-Type: application/json" -d ""
        sleep 5
    done
done

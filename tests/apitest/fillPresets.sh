#!/usr/bin/env bash
# get data from http://ctag-tbd.local/api/v1/getPlugins and store in json variable
json=$(curl -X GET "http://ctag-tbd.local/api/v1/getPlugins" -H "accept: application/json")
# parse json variable and store all id elements in array
ids=($(echo $json | jq -r '.[] | .id'))
# echo "ids: ${ids[@]}"
# iterate ids and send a get request to http://ctag-tbd.local/api/v1/setActivePlugin/0?id= where id is the current id
for id in "${ids[@]}"
do
    curl -X GET "http://ctag-tbd.local/api/v1/setActivePlugin/0?id=$id" -H "accept: application/json"
    # output http response code
    echo "Set plugin response code: $?"
    # send a get request to http://ctag-tbd.local/api/v1/savePreset/0?number=&name= and loop though number 0..9 and set the name to preset0..preset9
    for i in {0..9}
    do
        echo "id: $id i: $i"
        curl -X GET "http://ctag-tbd.local/api/v1/savePreset/0?number=$i&name=preset$i" -H "accept: application/json"
        # output http response code
        echo "Save preset response code: $?"
    done
done


# send a get request to http://ctag-tbd.local/api/v1/savePreset/0?number=0&name=cool0
#curl -X GET "http://ctag-tbd.local/api/v1/savePreset/0?number=0&name=coolx" -H "accept: application/json"
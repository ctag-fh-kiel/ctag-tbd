#!/usr/bin/env bash

# create an empty list
pluginList=()

# iterate all ctagSoundProcessor*.cpp files without ctagSoundProcessor.cpp in ../components/ctagSoundProcessor
for file in $(find ../../components/ctagSoundProcessor -name "ctagSoundProcessor*.cpp" ! -name "ctagSoundProcessor.cpp"); do
    # find out if within file there is the variable "isStereo" true or false
    isStereo=$(grep -q "isStereo\s*=\s*false" $file && echo false || echo true)
    # get the classname
    classname=$(basename $file .cpp)
    # strip ctagSoundProcessor from classname
    classname=${classname:18}
    # add classname and isStereo to pluginList
    pluginList+=("$classname $isStereo")
done

# divide pluginList into two lists, one for stereo and one for mono, remove isStereo from lists
pluginListStereo=($(printf "%s\n" "${pluginList[@]}" | grep "true" | cut -d " " -f 1))
pluginListMono=($(printf "%s\n" "${pluginList[@]}" | grep "false" | cut -d " " -f 1))

echo Starting plugin iteration
echo Beginning with Stereo plugins

# iterating all stereo plugins
# send a GET http://ctag-tbd.local/api/v1/setActivePlugin/0?id= request for each plugin in pluginListStereo
for i in "${!pluginListStereo[@]}"; do
    # echo the plugin name and how many plugins are left of all in the list
    echo Plugin ${pluginListStereo[$i]}
    curl -X GET "http://ctag-tbd.local/api/v1/setActivePlugin/0?id=${pluginListStereo[$i]}" -H "accept: application/json" -H "Content-Type: application/json" -d ""
    sleep 2
done

echo Beginning with Mono plugins
# iterating all mono plugins and combinations between them
# do the same with all mono plugins
for i in "${!pluginListMono[@]}"; do
    echo
    curl -X GET "http://ctag-tbd.local/api/v1/setActivePlugin/0?id=${pluginListMono[$i]}" -H "accept: application/json" -H "Content-Type: application/json" -d ""
    # iterate all mono plugins except the currently selected one and send a GET http://ctag-tbd.local/api/v1/setActivePlugin/1?id= request for each plugin in pluginListMono
    for j in "${!pluginListMono[@]}"; do
        if [ $i != $j ]; then
            echo CH0 ${pluginListMono[$i]} with CH1 ${pluginListMono[$j]}
            curl -X GET "http://ctag-tbd.local/api/v1/setActivePlugin/1?id=${pluginListMono[$j]}" -H "accept: application/json" -H "Content-Type: application/json" -d ""
            sleep 2
        fi
    done
    sleep 2
done

echo Test completed!
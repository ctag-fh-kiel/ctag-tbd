#!/usr/bin/env bash

# Configuration
HOST="ctag-tbd2.local"

# Function to send GET request with retry logic
send_request_with_retry() {
    local url="$1"
    local max_retries=999  # effectively infinite retries
    local retry_count=0
    local success=false

    while [ "$success" = false ]; do
        # Send the request with a timeout of 30 seconds max, 10 seconds to connect
        if curl -X GET "$url" -H "accept: application/json" -H "Content-Type: application/json" -d "" --connect-timeout 10 --max-time 30 --fail --silent --show-error; then
            success=true
            echo " ✓"
        else
            retry_count=$((retry_count + 1))
            echo " ✗ Request failed (attempt $retry_count), retrying in 5s..."
            sleep 5
        fi
    done
}

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

# randomize sort lists
pluginListStereo=($(shuf -e "${pluginListStereo[@]}"))
pluginListMono=($(shuf -e "${pluginListMono[@]}"))

# print pluginListStereo and pluginListMono
echo Stereo plugins: ${pluginListStereo[@]}
echo Mono plugins: ${pluginListMono[@]}
echo Starting plugin iteration
echo Beginning with Stereo plugins

doStereo=true
# if doStereo execute the next loop
if [ $doStereo = true ]; then
    # iterating all stereo plugins
    # send a GET http://ctag-tbd2.local/api/v1/setActivePlugin/0?id= request for each plugin in pluginListStereo
    for i in "${!pluginListStereo[@]}"; do
        # echo the plugin name and how many plugins are left of all in the list
        echo -n "Plugin ${pluginListStereo[$i]}: "
        send_request_with_retry "http://${HOST}/api/v1/setActivePlugin/0?id=${pluginListStereo[$i]}"
        sleep 2
    done
fi

echo Beginning with Mono plugins
# iterating all mono plugins and combinations between them
# do the same with all mono plugins
for i in "${!pluginListMono[@]}"; do
    echo -n "CH0 ${pluginListMono[$i]}: "
    send_request_with_retry "http://${HOST}/api/v1/setActivePlugin/0?id=${pluginListMono[$i]}"
    # iterate all mono plugins except the currently selected one and send a GET http://ctag-tbd2.local/api/v1/setActivePlugin/1?id= request for each plugin in pluginListMono
    for j in "${!pluginListMono[@]}"; do
          echo -n "  CH0 ${pluginListMono[$i]} with CH1 ${pluginListMono[$j]}: "
          send_request_with_retry "http://${HOST}/api/v1/setActivePlugin/1?id=${pluginListMono[$j]}"
    done
    sleep 2
done

echo Test completed!
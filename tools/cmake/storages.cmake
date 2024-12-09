include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/build_config/toolchains.cmake)


set(TBD_FILE_SYSTEM_RESOURCES "${CMAKE_SOURCE_DIR}/resources/file_system")

function(tbd_file_system_file_list)
    cmake_parse_arguments(arg "WEB_APP;PLUGINS" "" "" ${ARGN})

    if(arg_WEB_APP)
        set(web_app_dir "${TBD_FILE_SYSTEM_RESOURCES}/www")
        file(GLOB_RECURSE web_app_files RELATIVE "${web_app_dir}"
                "${web_app_dir}/*.html"
                "${web_app_dir}/*.js"
                "${web_app_dir}/*.css"
                "${web_app_dir}/*.ico"
        )
        foreach(file_path IN LISTS web_app_files)
            list(APPEND file_mapping "www/${file_path}" "www/${file_path}")
        endforeach()
    endif()

    if(arg_PLUGINS)
        set(plugin_dir "${TBD_FILE_SYSTEM_RESOURCES}/data")
        file(GLOB_RECURSE plugin_files RELATIVE "${plugin_dir}"
                "${plugin_dir}/*.jsn"
        )
        foreach(file_path IN LISTS plugin_files)
            list(APPEND file_mapping "data/${file_path}" "data/${file_path}")
            list(APPEND file_mapping "data/${file_path}" "dbup/${file_path}")
        endforeach()
    endif()

    tbd_store_or_return("${file_mapping}" ${ARGN})
endfunction()


# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
function(tbd_file_system_create_image)
    tbd_file_system_file_list(${ARGN} VAR file_mapping)

    while(file_mapping)
        list(POP_FRONT file_mapping src dest)
        set(src "${TBD_FILE_SYSTEM_RESOURCES}/${src}")
        set(dest "${CMAKE_BINARY_DIR}/spiffs_image/${dest}")
        configure_file("${src}" "${dest}" COPYONLY)
    endwhile()
    littlefs_create_partition_image(storage ${CMAKE_BINARY_DIR}/spiffs_image FLASH_IN_PROJECT)
endfunction()


function(tbd_storage_check)
    if ("${TBD_STORAGE_SAMPLES_ADDRESS}" STREQUAL "")
        tbd_loge("TBD_STORAGE_SAMPLES_ADDRESS not set, did you forget to call 'tbd_storages_setup'?")
    endif()
endfunction()


macro(tbd_storages_activate)
    tbd_toolchain_is_set()

    if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
        if ("${SDKCONFIG}" STREQUAL "")
            tbd_loge("no sdkconfig specified")
        endif()
        set(sdkconfig_file "${SDKCONFIG}")

        tbd_get_from_config_file("${sdkconfig_file}" CONFIG_SAMPLE_ROM_START_ADDRESS 
            VAR address)
        if ("${address}" STREQUAL "")
            tbd_logw("no sample offset present in sdkconfig")
        endif()
        tbd_set_param(TBD_STORAGE_SAMPLES_ADDRESS "${address}")

        tbd_file_system_create_image(WEB_APP PLUGINS)
    elseif ("${TBD_TOOLCHAIN}" STREQUAL "desktop")

        tbd_set_param(TBD_STORAGE_SAMPLES_ADDRESS 0)
    else()
        tbd_loge("toolchain does not support storages: '${TBD_TOOLCHAIN}'")
    endif()
endmacro()

#if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
#
#    file(GLOB_RECURSE WWW_FILES RELATIVE ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_SOURCE_DIR}/spiffs_image/www/*)
#    foreach(WWW_FILE ${WWW_FILES})
#        list(APPEND GZIP_COMMANDS
#                COMMAND gzip -9 ${CMAKE_BINARY_DIR}/spiffs_image/${WWW_FILE} )
#    endforeach()
#    if(NOT CONFIG_TBD_PLATFORM_BBA)
#        list(APPEND DEL_COMMANDS
#                COMMAND rm -f ${CMAKE_BINARY_DIR}/spiffs_image/data/sp/*DrumRack.jsn)
#        list(APPEND DEL_COMMANDS
#                COMMAND rm -f ${CMAKE_BINARY_DIR}/spiffs_image/dbup/sp/*DrumRack.jsn)
#    endif()
#    add_custom_target(copy-files ALL DEPENDS ${CMAKE_SOURCE_DIR}/spiffs_image
#            # clean up
#            COMMAND rm -rf ${CMAKE_BINARY_DIR}/spiffs_image
#            # copy spiffs files to build folder
#            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_BINARY_DIR}/spiffs_image
#            # create gzip encoded www files
#            ${GZIP_COMMANDS}
#            # and create backup file structure (sometimes when the ESP's power is interrupted during flash writes, the SPIFFS file gets corrupted)
#            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image/data ${CMAKE_BINARY_DIR}/spiffs_image/dbup
#            ${DEL_COMMANDS}
#    )
#
#    littlefs_create_partition_image(storage ${CMAKE_BINARY_DIR}/spiffs_image FLASH_IN_PROJECT DEPENDS copy-files)
#
#endif()

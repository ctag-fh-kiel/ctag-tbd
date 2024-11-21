include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/toolchains.cmake)

function(tbd_storage_check)
    if ("${TBD_STORAGE_SAMPLES_ADDRESS}" STREQUAL "")
        tbd_loge("TBD_STORAGE_SAMPLES_ADDRESS not set, did you forget to call 'tbd_storages_setup'?")
    endif()
endfunction()


macro(tbd_storages_setup)
    tbd_toolchain_is_set()

    if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
        set(sdkconfig_file "${CMAKE_SOURCE_DIR}/sdkconfig")
        tbd_get_from_config_file("${sdkconfig_file}" CONFIG_SAMPLE_ROM_START_ADDRESS 
            VAR address)
        if ("${address}" STREQUAL "")
            tbd_logw("no sample offset present in sdkconfig")
        endif()
        tbd_set_param(TBD_STORAGE_SAMPLES_ADDRESS "${address}")
    elseif ("${TBD_TOOLCHAIN}" STREQUAL "desktop")

        tbd_set_param(TBD_STORAGE_SAMPLES_ADDRESS 0)
    else()
        tbd_loge("toolchain does not support storages: '${TBD_TOOLCHAIN}'")
    endif()
endmacro()

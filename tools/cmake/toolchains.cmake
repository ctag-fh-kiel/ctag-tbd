include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/platforms.cmake)

# check if toolchain has been set up
#
# `TBD_TOOLCHAIN` has to refer to a valid toolhchaein name.
# 
# @note: Use `tbd_toolchain_setup` to set up toolchain for platform preset.
#
macro(tbd_toolchain_is_set)
    if ("${TBD_TOOLCHAIN}" STREQUAL "esp32" 
        OR "${TBD_TOOLCHAIN}" STREQUAL "desktop")
        # all is well
    else()
        tbd_loge("TBD_TOOLCHAIN not set, did you forget to invoke 'tbd_setup_toolchain_property'?")
    endif()
endmacro()

# diplay some information about the active toolchain
#
#
function(tbd_toolchain_print_info)
    message("
TBD toolchain configuration
---------------------------
TOOLCHAIN: ${TBD_TOOLCHAIN}
VERSION: ${TBD_TOOLCHAIN_VERSION}
---------------------------
        ")
endfunction()

# set up a tool chain for a given platform preset name
# 
# Ensures the `TBD_TOOLCHAIN` property is globally set. This should be one of the first
# things in your main `CMakeLists.txt` file.
#
# @arg platform_name   name of known platform preset
# 
# @warning: Does not activate the toolchain.
# @warning: only call once in project
#
macro(tbd_toolchain_setup platform_name)
    tbd_logv("determining toolchain for ${platform_name}")
    if ("${platform_name}" STREQUAL "")
        tbd_logW("TBD_PLATFORM not set, add '-DTBD_PLATFORM=<platform>' to idf.py or cmake")
    endif()

    tbd_platform_from_preset(${platform_name} VAR _platform)
    tbd_platform_system("${_platform}")
    tbd_logv("system for preset: ${_return}")
    if ("${_return}" STREQUAL "esp32")
        set(TBD_TOOLCHAIN "esp32" CACHE STRING "" FORCE)
        mark_as_advanced(TBD_TOOLCHAIN)

        include($ENV{IDF_PATH}/tools/cmake/version.cmake)
        set(TBD_TOOLCHAIN_VERSION 
            "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}"
            CACHE STRING "" FORCE)
        mark_as_advanced(TBD_TOOLCHAIN_VERSION)

        tbd_logv("using toolchain 'esp32'")
    elseif("${_return}" STREQUAL "desktop")
        set(TBD_TOOLCHAIN "desktop" CACHE STRING "" FORCE)
        mark_as_advanced(TBD_TOOLCHAIN)

        tbd_logv("using toolchain 'desktop'")
    else()
        tbd_loge("toolchain not supported: '${_return}'")
    endif()

    tbd_platform_arch("${_platform}")
    if ("${_return}" STREQUAL "")
        tbd_loge("invalid architecture ${_return}")
    endif()
    
    set(TBD_ARCH "${_return}" CACHE STRING "" FORCE)
    mark_as_advanced(TBD_ARCH)

endmacro()


# activate a toolchain
# 
# Usually you would call `tbd_toolchain_setup` before activating toolchain, to set toolchain
# parameters.
#
# In the case of ESP IDF the idf build tools are imported and activated including component
# discovery.
#
# @warning: requires `TBD_TOOLCHAIN` to be set
# @warning: only call once in project
#
macro(tbd_toolchain_activate)
    tbd_toolchain_is_set()

    if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
        set(SDKCONFIG_DEFAULTS "${CMAKE_SOURCE_DIR}/config/esp_sdkconfigs/sdkconfig.defaults.${TBD_PLATFORM}")
        set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig.${TBD_PLATFORM}")
        set(IDF_TARGET "${TBD_ARCH}")

        include($ENV{IDF_PATH}/tools/cmake/project.cmake)
        list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/ports/tbd_port_esp32)

        # make project globals available to ESP IDF components
        idf_build_set_property(IDF_VERSION ${TBD_TOOLCHAIN_VERSION})
        idf_build_set_property(TBD_TOOLCHAIN ${TBD_TOOLCHAIN})
        idf_build_set_property(TBD_PLATFORM ${TBD_PLATFORM})
        idf_build_set_property(TBD_PLATFORM_OBJ "${TBD_PLATFORM_OBJ}")

        tbd_set_param(TBD_DRIVERS_PORT_LIB __idf_tbd_drivers_port)    
        tbd_set_param(TBD_SYSTEM_PORT_LIB __idf_tbd_system_port)      
        tbd_set_param(TBD_STORAGE_PORT_LIB __idf_tbd_storage_port)  
        tbd_set_param(TBD_DSP_LIB __idf_espressif__esp-dsp)
        tbd_set_param(TBD_DISPLAY_PORT_LIB __idf_tbd_display_port)
    elseif ("${TBD_TOOLCHAIN}" STREQUAL "desktop")
        tbd_set_param(TBD_DRIVERS_PORT_LIB tbd_drivers_port)  
        tbd_set_param(TBD_SYSTEM_PORT_LIB tbd_system_port)   
        tbd_set_param(TBD_STORAGE_PORT_LIB tbd_storage_port) 
        tbd_set_param(TBD_DSP_LIB tbd_dsp) 
        tbd_set_param(TBD_DISPLAY_PORT_LIB tbd_display_port)
    else()
        tbd_loge("toolchain can not be activated: '${TBD_TOOLCHAIN}'")
    endif()
    tbd_toolchain_print_info()
endmacro()


# set global TBD compilation defines
#
# Put this in the compiler configuration of your main `CMakeLists.txt`
#
# Make sure to call this when setting up the compiler
#
function (tbd_toolchain_add_defines)
    tbd_logv("adding compilation defines")

    tbd_platform_check()
    tbd_platform_get_features("${TBD_PLATFORM_OBJ}")   
    set(defs ${_return})
    add_compile_definitions(${defs})

    # if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
    #     idf_build_set_property(TBD_GLOBAL_FLAGS ${defs} APPEND)
    #     idf_build_set_property(TBD_GLOBAL_FLAGS RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 APPEND)
    #     idf_build_set_property(TBD_GLOBAL_FLAGS -DRAPIDJSON_HAS_STDSTRING=1 APPEND)
    # endif()
endfunction()


# ensure global compiler options are set properly for toolchain
#
# Put this in the compiler configuration of your main `CMakeLists.txt`
#
# @note: ensures esp32 components and plain libraries are compiled using the same options
#
macro(tbd_toolchain_add_options) 
    tbd_logv("adding compile options")

    if("${TBD_TOOLCHAIN}" STREQUAL "esp32") 
        idf_build_set_property(COMPILE_OPTIONS -Wno-unused-local-typedefs -ffast-math APPEND) # -ffast-math -fno-finite-math-only https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity    

        # FIXME: copying idf compiler options causes errors
        #
        # add_compile_options(${TBD_COMPILE_OPTIONS})
        # idf_build_get_property(TBD_COMPILE_OPTIONS COMPILE_OPTIONS)
        # foreach(opt ${TBD_COMPILE_OPTIONS})
        #     message("IDF compiler option ${opt}")
        # endforeach()

        # HOTFIX: manual compiler options
        add_compile_options(
            -fno-exceptions 
            -O2 

            # disable C++ dynamic features
            -fno-rtti 

            # enable
            -mlongcalls  
            -ffast-math 

            # disable
            -mdisable-hardware-atomics 
            -fstrict-volatile-bitfields 
            -fno-jump-tables 
            -fno-tree-switch-conversion 


            # binary layout
            -ffunction-sections 
            -fdata-sections 

            # debugger
            -gdwarf-4 
            -ggdb 

            # disable certain stdlib features
            -fno-builtin-memcpy 
            -fno-builtin-memset 
            -fno-builtin-bzero 
            -fno-builtin-stpcpy 
            -fno-builtin-strncpy 

            # errors and warnings
            -fdiagnostics-color=always 
            -Wall 
            -Werror=all 
            -Wextra

            -Wno-error=unused-function 
            -Wno-error=unused-variable 
            -Wno-error=unused-but-set-variable 
            -Wno-error=deprecated-declarations 
            -Wno-unused-parameter 
            -Wno-sign-compare 
            -Wno-enum-conversion 
            -Wno-unused-local-typedefs
        )
    elseif ("${TBD_TOOLCHAIN}" STREQUAL "desktop")
        # add_compile_options(-stdlib=libc++ -lc++abi)
    else()
        tbd_log(FATAL_ERROR "invalid toolchain ${TBD_TOOLCHAIN}")
    endif()
endmacro()

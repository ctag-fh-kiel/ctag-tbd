if(TBD_SIM)
    # stub empty component for tbd simulator

else()
    idf_component_register(
            SRCS "link.cpp"
            INCLUDE_DIRS "link/include" "."
            REQUIRES asio
            PRIV_REQUIRES sock_utils esp_timer esp_netif
    )
    if(CONFIG_ABLETON_LINK)
        message("Ableton Link Enabled")
        target_compile_features(${COMPONENT_LIB} PRIVATE cxx_generalized_initializers)
        # link task on core0
        target_compile_definitions(${COMPONENT_LIB} PRIVATE LINK_ESP_TASK_CORE_ID=0)
        target_compile_options(${COMPONENT_LIB} PRIVATE
                -Wno-return-type
                -Wno-multichar
                -Wno-deprecated-declarations
                -Wno-redundant-move
        )
        endif()
endif()
add_subdirectory(rapidjson)
add_subdirectory(mutable)
add_subdirectory(moog)

# FIXME: consider building a shared lib for desktop builds
set(RTAUDIO_BUILD_STATIC_LIBS TRUE)
add_subdirectory(rtaudio)
cmake_minimum_required(VERSION 3.0.0)
project(demo VERSION 0.1.0)


# set the path to the prebuilt webrtc lib, available from 
# https://github.com/shiguredo-webrtc-build/webrtc-build/releases
set(LIBWEBRTC_INCLUDE_PATH /path/to/prebuilt/webrtc/include)
set(LIBWEBRTC_BINARY_PATH /path/to/prebuilt/webrtc/lib)
#for example, yan can set: 
#   set(LIBWEBRTC_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/third_party/webrtc/include)
#   set(LIBWEBRTC_BINARY_PATH ${PROJECT_SOURCE_DIR}/third_party/webrtc/lib)
# when the files follow this: 
#   ── third_party
#         ├
#         webrtc
#         ├── include
#         │   ├── api
#         │   ├── audio
#         │   ├── base
#         │   ...    
#         ├── lib
#         │   ├── libboringssl.a
#         │   └── libwebrtc.a
#         ├── libwebrtc-linux-x64.tar.xz
#         ├── NOTICE
#         └── VERSION


# source files of datachannel demo
set(
    SOURCE_FILES_DATACHANNEL
    src/datachannel/main.cpp
)

# source files of video demo 
set(
    SOUCE_FILES_VIDEO
    src/video-channel/main.cpp
    src/video-channel/i420_creator.cpp
    src/video-channel/i420_creator.h
)

# source files of audio demo
set(
    SOUCE_FILES_AUDIO
    src/audio-channel/main.cpp
)


# datachannel demo
add_executable(demo-data "${SOURCE_FILES_DATACHANNEL}")
target_include_directories(
    demo-data PUBLIC
    "${LIBWEBRTC_INCLUDE_PATH}"
	"${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp"
)
target_link_libraries(
    demo-data 
    ${LIBWEBRTC_BINARY_PATH}/libwebrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
    pthread
    dl
)
target_compile_definitions(demo-data PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)


#video demo
add_executable(demo-video "${SOUCE_FILES_VIDEO}")
target_include_directories(
    demo-video PUBLIC
    "${LIBWEBRTC_INCLUDE_PATH}"
	"${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp"
)
target_link_libraries(
    demo-video 
    ${LIBWEBRTC_BINARY_PATH}/libwebrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
    pthread
    dl
)
target_compile_definitions(demo-video PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)


#audio demo
add_executable(demo-audio "${SOUCE_FILES_AUDIO}")
target_include_directories(
    demo-audio PUBLIC
    "${LIBWEBRTC_INCLUDE_PATH}"
	"${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp"
)
target_link_libraries(
    demo-audio 
    ${LIBWEBRTC_BINARY_PATH}/libwebrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
    pthread
    dl
)

# Compile definitions for libwebrtc: -DWEBRTC_POSIX
target_compile_definitions(demo-audio PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)


PROJECT(demo.bin)

INCLUDE_DIRECTORIES(
    ${TOP_SOURCE_DIR}/demo/dec_g711
    ${TOP_SOURCE_DIR}/demo/src
    ${TOP_SOURCE_DIR}/libs/ffmpeg/include
)

LINK_DIRECTORIES(
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    ${TOP_SOURCE_DIR}/libs/ffmpeg/lib
    ${TOP_SOURCE_DIR}/libs/fdk-aac/lib
)

AUX_SOURCE_DIRECTORY(. SRC_LIST)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_LIST})

TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME} 

    #-static
    -Wl,--start-group
    -lrt -lpthread -lm -ldl
    -ldec_g711

    -lfdk-aac
    -lavformat   
    -lavcodec
    -lavdevice
    -lavfilter
    -lavutil
    -lswresample
    -lswscale

    -Wl,--end-group
)



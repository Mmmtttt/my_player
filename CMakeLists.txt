cmake_minimum_required(VERSION 3.0)
project(MyPlayer)

# 设置源代码目录
set(SOURCES
    main.cpp
    #main1.cpp
    #main2.cpp
    #main3.c
    #main4.cpp
    #main5.cpp
    #ffplayer.c
    #tutorial03.c
    src/decoder.cpp
    src/frame.cpp
    src/renderer.cpp
    src/video.cpp
    src/packetQueue.cpp
    # 添加其他源文件
)

# 设置头文件目录
include_directories(include 
${CMAKE_SOURCE_DIR}/libs_win/develop/ffmpeg-20181212/include 
${CMAKE_SOURCE_DIR}/libs_win/develop/SDL2-2.0.9/include
)

# 添加 FFmpeg 库目录
#link_directories(libs)
link_directories (
    ${CMAKE_SOURCE_DIR}/libs_win/develop/ffmpeg-20181212/lib 
    ${CMAKE_SOURCE_DIR}/libs_win/develop/SDL2-2.0.9/lib/x64
    ${CMAKE_SOURCE_DIR}/libs_win/runtime
    )


# 添加可执行文件生成
add_executable(MyPlayer ${SOURCES})


set_target_properties(MyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 链接 FFmpeg 库
target_link_libraries(MyPlayer
    avcodec.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    SDL2main.lib
    SDL2.lib
)

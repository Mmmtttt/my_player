cmake_minimum_required(VERSION 3.0)
project(MyPlayer)

# 设置源代码目录
set(SOURCES
    #main.cpp
    #main1.cpp
    #main2.cpp
    #main3.c
    main4.cpp
    #main5.cpp
    #ffplayer.c
    #tutorial03.c
    #src/decoder.cpp
    #src/frame.cpp
    #src/renderer.cpp
    #src/video.cpp
    # 添加其他源文件
)

# 设置头文件目录
include_directories(include 
C:/Users/86156/Desktop/player/libs_win/develop/ffmpeg-20181212/include 
C:/Users/86156/Desktop/player/libs_win/develop/SDL2-2.0.9/include
)

# 添加 FFmpeg 库目录
#link_directories(libs)
link_directories (
    C:/Users/86156/Desktop/player/libs_win/develop/ffmpeg-20181212/lib 
    C:/Users/86156/Desktop/player/libs_win/develop/SDL2-2.0.9/lib/x64
    )


# 添加可执行文件生成
add_executable(MyPlayer ${SOURCES})


set_target_properties(MyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 链接 FFmpeg 库
target_link_libraries(MyPlayer
    avcodec.dll
    avformat.dll
    avutil.dll
    swresample.dll
    swscale.dll
    SDL2.dll
    SDL2main.dll
    SDL2.dll
    avfilter.dll
    avdevice.dll
)

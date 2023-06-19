cmake_minimum_required(VERSION 3.0)
project(MyPlayer)

# 设置源代码目录
set(SOURCES
    main.cpp

    src/decoder.cpp
    src/frame.cpp
    src/renderer.cpp
    src/video.cpp
    # 添加其他源文件
)

# 设置头文件目录
include_directories(include J:/transform/ffmpeg-n6.0-latest-win64-lgpl-shared-6.0/include J:/transform/SDL2-2.0.16/include)

# 添加 FFmpeg 库目录
#link_directories(libs)
link_directories (J:/transform/ffmpeg-n6.0-latest-win64-lgpl-shared-6.0/lib J:/transform/SDL2-2.0.16/lib/x64)


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
    SDL2.lib
    SDL2main.lib
    SDL2.dll
)

cmake_minimum_required(VERSION 3.0)
project(MyPlayer)

# 设置源代码目录
set(PLAYER
    #main.cpp
    #src/audio.cpp
    src/player/decoder.cpp
    src/player/frame.cpp
    src/player/renderer.cpp
    src/player/video.cpp
    src/player/packetQueue.cpp
    src/player/audio.cpp
    src/player/control.cpp
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
add_executable(MyPlayer ${PLAYER} main.cpp src/connect/win_net.cpp)
add_executable(serverMyPlayer ${PLAYER} src/connect/server.cpp src/connect/win_net.cpp)
add_executable(clientMyPlayer ${PLAYER} src/connect/client.cpp src/connect/win_net.cpp)


set_target_properties(MyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(serverMyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(clientMyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 链接 FFmpeg 库
target_link_libraries(MyPlayer
    avcodec.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    SDL2main.lib
    SDL2.lib
    ws2_32
)
target_link_libraries(serverMyPlayer
    avcodec.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    SDL2main.lib
    SDL2.lib
    ws2_32
)
target_link_libraries(clientMyPlayer
    avcodec.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    SDL2main.lib
    SDL2.lib
    ws2_32
)

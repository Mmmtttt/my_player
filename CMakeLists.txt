cmake_minimum_required(VERSION 3.5)
project(MyPlayer)

# Qt相关
set(CMAKE_AUTOUIC ON)
# Qt's Meta-Object Compiler
set(CMAKE_AUTOMOC ON)
# Qt资源系统
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(CMAKE_BUILD_TYPE Debug)

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
    src/connect/win_net.cpp
    src/connect/connect.cpp
    src/connect/transport_session.cpp
    src/portocol/my_portocol.cpp
    
    
    libs_win/develop/json.hpp
    # 添加其他源文件
)

set(HOMEPAGE
    src/player/player.cpp
    src/homepage/mainwindow.cpp
    src/homepage/permissionwindow.cpp
    src/homepage/settingswindow.cpp
    src/homepage/userwindow.cpp
)

# 设置头文件目录
include_directories(
    include 
    include/UI
    build
    ${CMAKE_SOURCE_DIR}/libs_win/develop/ffmpeg-20181212/include 
    ${CMAKE_SOURCE_DIR}/libs_win/develop/SDL2-2.0.9/include
    ${CMAKE_SOURCE_DIR}/libs_win/develop
)

# 添加 FFmpeg 库目录
#link_directories(libs)
link_directories (
    ${CMAKE_SOURCE_DIR}/libs_win/develop/ffmpeg-20181212/lib 
    ${CMAKE_SOURCE_DIR}/libs_win/develop/SDL2-2.0.9/lib/x64
    ${CMAKE_SOURCE_DIR}/libs_win/runtime
    )

set(MOCFILES
    include/player.h
    include/UI/mainwindow.h
    include/UI/settingswindow.h
    include/UI/permissionwindow.h
    include/UI/userwindow.h )
qt6_wrap_cpp(MOC_SOURCES ${MOCFILES})


set(UICFILES
    src/homepage/mainwindow.ui
    src/homepage/settingswindow.ui
    src/homepage/permissionwindow.ui
    src/homepage/userwindow.ui)
qt6_wrap_ui(UI_HEADERS ${UICFILES})


# 添加可执行文件生成
add_executable(MyPlayer ${PLAYER} main.cpp )
add_executable(serverMyPlayer ${PLAYER} src/connect/server.cpp )
add_executable(clientMyPlayer ${PLAYER} src/connect/client.cpp )
add_executable(Main2 
    ${PLAYER} 
    ${UI_HEADERS}
    main2.cpp 
    ${HOMEPAGE}
    ${MOC_SOURCES}
)


set_target_properties(MyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(serverMyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(clientMyPlayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(Main2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)



# 链接 FFmpeg 库
set(NET_FF_SDL_LIBRARIES
    avcodec.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    SDL2main.lib
    SDL2.lib
    ws2_32
)

target_link_libraries(MyPlayer PRIVATE
    ${NET_FF_SDL_LIBRARIES}
)
target_link_libraries(serverMyPlayer PRIVATE
    ${NET_FF_SDL_LIBRARIES}
)
target_link_libraries(clientMyPlayer PRIVATE
    ${NET_FF_SDL_LIBRARIES}
)
target_link_libraries(Main2 PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${NET_FF_SDL_LIBRARIES}
    Qt6::Core
)

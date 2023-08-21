@echo off
mkdir build
cd build
..\libs_win\CMake_64\bin\cmake -G "MinGW Makefiles" ..
mingw32-make -j4

@echo off
mkdir build
cd build
..\bin\CMake_64\bin\cmake -G "MinGW Makefiles" ..
mingw32-make -j4

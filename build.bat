@echo off

set cur_dir=%cd%
set PATH=%PATH%;%cur_dir%\libs_win\mingw\bin

mkdir build
cd build

..\libs_win\CMake_64\bin\cmake -G "MinGW Makefiles" ..
mingw32-make -j4

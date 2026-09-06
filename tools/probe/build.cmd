@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cl 2>&1
cmake -S tools/probe -B build/probe -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.7.2/msvc2019_64
if errorlevel 1 exit /b 1
cmake --build build/probe
if errorlevel 1 exit /b 1
ctest --test-dir build/probe --output-on-failure

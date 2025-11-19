@echo off
setlocal

set APP_NAME=keybinder
set BUILD_DIR=build
set DEPLOY_DIR=deploy\%APP_NAME%
set QT_VERSION=6.5.3
set QT_ARCH=msvc2019_64
set QT_INSTALL_DIR=C:\Qt\%QT_VERSION%\%QT_ARCH%

echo Setting up MSVC environment...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)
pushd %BUILD_DIR%

echo Configuring CMake...
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH=%QT_INSTALL_DIR% -DCMAKE_BUILD_TYPE=Release

echo Building project...
cmake --build . --config Release

popd

echo Build complete.
endlocal

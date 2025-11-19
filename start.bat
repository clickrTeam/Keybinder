@echo off
setlocal

set APP_NAME=keybinder
set BUILD_DIR=build

echo Running %APP_NAME%...
%BUILD_DIR%\Release\%APP_NAME%.exe

endlocal

@echo off
setlocal

echo Installing aqtinstall using uv...
uv pip install aqtinstall

echo Installing Qt 6.5.3 for Windows (MSVC2019_64) to C:\Qt...
aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -O C:\Qt

echo Qt installation complete.
endlocal

@echo off
setlocal enabledelayedexpansion

REM 
set SFML_DIR=C:\SFML-2.6.2
set VCPKG_DIR=C:\Users\USER\OneDrive\Desktop\vcpkg\installed\x64-mingw-dynamic
set SRC_DIR=src
set BIN_DIR=.

REM
if not exist "%SRC_DIR%" mkdir "%SRC_DIR%"

REM 
echo Compiling flight_booking.cpp...
g++ -std=c++17 ^
 -I. ^
 -I"%VCPKG_DIR%\include" ^
 -L"%VCPKG_DIR%\lib" ^
 -o "%BIN_DIR%\flight_booking.exe" "%SRC_DIR%\flight_booking.cpp" ^
 -lcpr -lcurl -lssl -lcrypto -lzlib -lws2_32 -lwsock32
if %ERRORLEVEL% NEQ 0 (
    echo  Error compiling flight_booking.cpp
    pause
    exit /b 1
)

REM 
echo Compiling flight_simulator.cpp...
g++ -std=c++17 ^
 -I. ^
 -I"%VCPKG_DIR%\include" ^
 -I"%SFML_DIR%\include" ^
 -L"%VCPKG_DIR%\lib" ^
 -L"%SFML_DIR%\lib" ^
 -o "%BIN_DIR%\flight_simulator.exe" "%SRC_DIR%\flight_simulator.cpp" ^
 -lcpr -lcurl -lssl -lcrypto -lzlib ^
 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio ^
 -lws2_32 -lwsock32
if %ERRORLEVEL% NEQ 0 (
    echo  Error compiling flight_simulator.cpp
    pause
    exit /b 1
)

REM 
echo Copying required DLLs...
if not exist "dll" mkdir "dll"
copy "%VCPKG_DIR%\bin\*.dll" "dll\" /Y
copy "%SFML_DIR%\bin\*.dll" "dll\" /Y

REM 
echo.
echo Compilation and setup successful!
echo.
pause

@echo off

setlocal

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rem Get the directory of the batch file
set "SCRIPT_DIR=%~dp0"
rem Remove trailing backslash
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

rem Get normalized parent directory
for %%I in ("%SCRIPT_DIR%\..") do set "SOURCE_DIR=%%~fI"

set "EXTERN_DIR=%SOURCE_DIR%\build\extern_deps"
set "BUILD_DIR=%SOURCE_DIR%\build\windows"

echo Bootstrapping external dependencies...
cmake -G "Ninja" -S %SOURCE_DIR% -B %EXTERN_DIR% -DWF_BOOTSTRAP=ON -DCMAKE_BUILD_TYPE=Release
cmake --build %EXTERN_DIR%

echo Building wayfinder...
cmake -G "Visual Studio 17 2022" -S %SOURCE_DIR% -B %BUILD_DIR% -DWF_BOOTSTRAP=OFF
cmake --build %BUILD_DIR% --config RelWithDebInfo
endlocal

exit
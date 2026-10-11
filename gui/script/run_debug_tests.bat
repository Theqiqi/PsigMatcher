@echo off
setlocal
set GIT_SSL_NO_VERIFY=1

rem Qt path -- set your own if using a different location
if "%QT6_DEBUG_DIR%"=="" set "QT6_DEBUG_DIR=C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug"

rem Shared Qt: add Qt bin to PATH so Qt6Cored.dll / plugins resolve at runtime
set "PATH=%QT6_DEBUG_DIR%/bin;%PATH%"

pushd "%~dp0.."
if not exist "out\build\debug" ( echo [run_debug_tests] not built yet & popd & exit /b 1 )

pushd out\build\debug
ctest --output-on-failure
set RC=%errorlevel%
popd
popd

if not "%RC%"=="0" ( echo [run_debug_tests] FAILED & exit /b %RC% )
echo [run_debug_tests] OK
exit /b 0

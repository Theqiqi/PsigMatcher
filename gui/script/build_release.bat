@echo off
setlocal
set GIT_SSL_NO_VERIFY=1

rem VCPKG_ROOT -- use VS2022 bundled vcpkg
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg"

rem Qt path -- set your own if using a different location
rem You can also set QT6_RELEASE_DIR as a persistent system/user environment variable
if "%QT6_RELEASE_DIR%"=="" set "QT6_RELEASE_DIR=C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64"

rem Initialize MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 ( echo [build_release] vcvars64 failed & exit /b 1 )

pushd "%~dp0.."
cmake --preset release
if errorlevel 1 ( echo [build_release] configure failed & popd & exit /b 1 )
cmake --build --preset release
if errorlevel 1 ( echo [build_release] build failed & popd & exit /b 1 )
popd
echo [build_release] OK
exit /b 0

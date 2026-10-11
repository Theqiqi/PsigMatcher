@echo off
setlocal
set GIT_SSL_NO_VERIFY=1

rem Static Qt Release: no Qt DLLs needed on PATH (everything linked statically /MT)
rem gtest is x64-windows-static as well, so no gtest DLLs either

pushd "%~dp0.."
if not exist "out\build\release" ( echo [run_release_tests] not built yet & popd & exit /b 1 )

pushd out\build\release
ctest --output-on-failure
set RC=%errorlevel%
popd
popd

if not "%RC%"=="0" ( echo [run_release_tests] FAILED & exit /b %RC% )
echo [run_release_tests] OK
exit /b 0

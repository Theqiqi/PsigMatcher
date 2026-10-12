@echo off
rem dump_directives.bat -- 抽查静态库 psig.lib 的 /DEFAULTLIB 指令，校验运行库策略
rem 用法: dump_directives.bat [Debug|Release]   默认 Release
setlocal
set CFG=%1
if "%CFG%"=="" set "CFG=Release"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 ( echo [dump_directives] vcvars64 failed & exit /b 1 )

set "LIB=%~dp0..\..\..\psig\out\lib\%CFG%\psig.lib"
if not exist "%LIB%" ( echo [dump_directives] missing %LIB% & exit /b 1 )

echo === dumpbin /directives psig.lib (%CFG%) ===
dumpbin /directives "%LIB%" | findstr /I "DEFAULTLIB"
exit /b 0

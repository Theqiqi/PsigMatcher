@echo off
rem dump_dependents.bat -- 输出 GUI 生产 exe 的 DLL 依赖，用于校验运行库链接策略
rem 用法: dump_dependents.bat [Debug|Release]   默认 Release
setlocal
set CFG=%1
if "%CFG%"=="" set "CFG=Release"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 ( echo [dump_dependents] vcvars64 failed & exit /b 1 )

set "EXE=%~dp0..\..\out\bin\%CFG%\PsigMatcher.exe"
if not exist "%EXE%" ( echo [dump_dependents] missing %EXE% & exit /b 1 )

echo === dumpbin /dependents %CFG%\PsigMatcher.exe ===
dumpbin /dependents "%EXE%"
exit /b 0

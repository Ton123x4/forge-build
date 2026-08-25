@echo off
@setlocal EnableExtensions EnableDelayedExpansion

set "ROOT_DIRECTORY=%~dp0.."

pushd "%ROOT_DIRECTORY%" >nul

set "FORGE_DIRECTORY=.forge"
set "BOOTSTRAP_DIRECTORY=%FORGE_DIRECTORY%\bootstrap"
set "BOOTSTRAP_LOGFILE=%BOOTSTRAP_DIRECTORY%\bootstrap.log"
set "FORGE_EXECUTABLE=%BOOTSTRAP_DIRECTORY%\forge.exe"

set "STDEXT_INCLUDE=packages/stdext/include"
set "CORE_INCLUDE=packages/forge-core/include"
set "FORGE_INCLUDE=packages/forge-build/include"
set "FORGE_SOURCES=packages/forge-build/src"
set "CORE_SOURCES=packages/forge-core/src"
set "STDEXT_SOURCES=packages/stdext/src"

rem --- Pick compiler (clang only) ---
set "CXX="
where clang >nul 2>&1
if not errorlevel 1 (
    set "CXX=clang"
)

if "%CXX%"=="" (
    echo :: No supported C++ compiler found in PATH.
    echo :: Install clang and try again.
    popd >nul
    exit /b 1
)

rem --- Ensure directories exist ---
if not exist "%FORGE_DIRECTORY%" (
    mkdir "%FORGE_DIRECTORY%" >nul 2>&1
)
if not exist "%BOOTSTRAP_DIRECTORY%" (
    mkdir "%BOOTSTRAP_DIRECTORY%" >nul 2>&1
)

rem --- Bootstrap build only if the executable does not exist ---
if not exist "%FORGE_EXECUTABLE%" (
    echo Bootstrap Folder:    %BOOTSTRAP_DIRECTORY%
    echo Bootstrap Log File:  %BOOTSTRAP_LOGFILE%
    echo.
    echo :: Bootstrapping Forge Build Tool...
    "%CXX%" -std=c++20 -stdlib=libstdc++ -Wno-c23-extensions -l ws2_32.lib -l Advapi32.lib -o "%FORGE_EXECUTABLE%" -I "%STDEXT_INCLUDE%" -I "%CORE_INCLUDE%" -I "%FORGE_INCLUDE%" "%FORGE_SOURCES%\*.cpp" "%CORE_SOURCES%\*.cpp" "%STDEXT_SOURCES%\*.cpp" > "%BOOTSTRAP_LOGFILE%" 2>&1
    if errorlevel 1 (
        echo :: Forge bootstrap build failed. See "%BOOTSTRAP_LOGFILE%".
        popd >nul
        exit /b 1
    )
    echo :: Forge bootstrap build OK
)

rem --- Safety: bare clean cannot be run from within the repo directory ---
rem It would remove the .forge directory this executable is currently running from.
if /i "%1"=="clean" (
    set "IS_BARE_CLEAN=1"

    for %%A in (%*) do (
        if /i "%%A"=="--debug"   set "IS_BARE_CLEAN="
        if /i "%%A"=="--preview" set "IS_BARE_CLEAN="
        if /i "%%A"=="--release" set "IS_BARE_CLEAN="
    )

    if defined IS_BARE_CLEAN (
        echo :: 'clean' without a profile cannot be run from this location. It would remove the .forge directory this executable is currently running from.
        popd >nul
        exit /b 1
    )
)

rem --- Run Forge with all provided args ---
"%FORGE_EXECUTABLE%" %*
if errorlevel 1 (
    echo :: Forge build failed with exit code %ERRORLEVEL%.
    popd >nul
    exit /b %ERRORLEVEL%
)

popd >nul
exit /b 0

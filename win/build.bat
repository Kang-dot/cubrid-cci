@echo off

REM
REM  Copyright 2008 Search Solution Corporation
REM  Copyright 2016 CUBRID Corporation
REM 
REM   Licensed under the Apache License, Version 2.0 (the "License");
REM   you may not use this file except in compliance with the License.
REM   You may obtain a copy of the License at
REM 
REM       http://www.apache.org/licenses/LICENSE-2.0
REM 
REM   Unless required by applicable law or agreed to in writing, software
REM   distributed under the License is distributed on an "AS IS" BASIS,
REM   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM   See the License for the specific language governing permissions and
REM   limitations under the License.
REM 

SETLOCAL

rem CUBRID build script for MS Windows.
rem
rem Requirements
rem - cmake
rem - default VS2017 (for windows)
rem - optional VS2015, VS2012 (for windows)
rem - Windows 2003 or later
rem - git 1.7.6 or later

if NOT "%OS%"=="Windows_NT" echo "ERROR: Not supported OS" & GOTO :EOF

rem clear ERRORLEVEL

set SCRIPT_DIR=%~dp0

rem set default value
set VERSION=0
set VERSION_FILE=BUILD_NUMBER
set BUILD_NUMBER=0
set BUILD_GENERATOR="Visual Studio 15 2017"
set BUILD_GEN_VERSION=V141
set BUILD_TARGET=x64
set BUILD_MODE=Release
set BUILD_TYPE=RelWithDebInfo
set CMAKE_PATH=C:\Program Files\CMake\bin\cmake.exe
set CPACK_PATH=C:\Program Files\CMake\bin\cpack.exe
set GIT_PATH=C:\Program Files\Git\bin\git.exe
set CCI_VERSION_START_DATE=2021-07-14
set OPTION_CHECK=false

rem default list is all
set BUILD_LIST=build
rem unset BUILD_ARGS
if NOT "%BUILD_ARGS%." == "." set BUILD_ARGS=

rem set variables
call :ABSPATH "%SCRIPT_DIR%\.." SOURCE_DIR

rem unset DIST_PKGS
if NOT "%DIST_PKGS%." == "." set DIST_PKGS=

:CHECK_OPTION
if "%~1." == "."       GOTO :BUILD
set BUILD_OPTION=%1
if "%~1" == "/32"         set BUILD_TARGET=Win32& set OPTION_CHECK=true
if "%~1" == "/64"         set BUILD_TARGET=x64& set OPTION_CHECK=true
if /I "%~1" == "/debug"   set "BUILD_MODE=Debug"& set BUILD_TYPE=Debug& set OPTION_CHECK=true
if /I "%~1" == "/release" set "BUILD_MODE=Release"& set BUILD_TYPE=RelWithDebInfo& set OPTION_CHECK=true
if /I "%~1" == "/vs2017"  set BUILD_GENERATOR="Visual Studio 15 2017"& set BUILD_GEN_VERSION=V141& set OPTION_CHECK=true
if /I "%~1" == "/vs2015"  set BUILD_GENERATOR="Visual Studio 14 2015"& set BUILD_GEN_VERSION=V140& set OPTION_CHECK=true
if /I "%~1" == "/vs2012"  set BUILD_GENERATOR="Visual Studio 11 2012"& set BUILD_GEN_VERSION=V110& set OPTION_CHECK=true
if "%~1" == "/h"          GOTO :SHOW_USAGE
if "%~1" == "/?"          GOTO :SHOW_USAGE
if "%~1" == "/help"       GOTO :SHOW_USAGE
if NOT "%BUILD_OPTION:~0,1%" == "/" (
  set BUILD_ARGS=%BUILD_ARGS% %1
) else if %OPTION_CHECK%==false (
  echo not found option [%BUILD_OPTION%]
  GOTO :SHOW_USAGE
)
shift
GOTO :CHECK_OPTION


:BUILD
if NOT "%BUILD_ARGS%." == "." set BUILD_LIST=%BUILD_ARGS%
:: Remove others if ALL found
set _TMP_LIST=%BUILD_LIST:ALL= %
if NOT "%_TMP_LIST%" == "%BUILD_LIST%" set BUILD_LIST=ALL

for /f "tokens=* delims= " %%a IN ("%BUILD_LIST%") DO set BUILD_LIST=%%a
echo Build list is [%BUILD_LIST%].
set BUILD_LIST=%BUILD_LIST:ALL=BUILD CCI_PACKAGE%
set BUILD_LIST=%BUILD_LIST:BUILD=CUBRID%

for %%i IN (%BUILD_LIST%) DO (
  echo.
  echo [%DATE% %TIME%] Entering target [%%i]
  call :BUILD_PREPARE
  if ERRORLEVEL 1 echo *** [%DATE% %TIME%] Preparing failed. & GOTO :EOF
  call :BUILD_%%i
  if ERRORLEVEL 1 echo *** [%DATE% %TIME%] Failed target [%%i] & GOTO :SHOW_USAGE
  echo [%DATE% %TIME%] Leaving target [%%i]
  echo.
)
echo.
echo [%DATE% %TIME%] Completed.
echo.
echo *** Summary ***
echo   Target [%BUILD_LIST%]
echo   Version [%VERSION%]
echo   Build mode [%BUILD_TARGET%/%BUILD_MODE%]
if NOT "%DIST_PKGS%." == "." (
  echo   Generated packages in [%DIST_DIR%]
  cd /d %DIST_DIR%
  for /f "delims=" %%i in ('md5sum -t %DIST_PKGS%') DO (
    echo     - %%i
  )
)
echo.
GOTO :EOF


:BUILD_PREPARE
echo Checking for requirements...
call :FINDEXEC cmake.exe CMAKE_PATH "%CMAKE_PATH%"
call :FINDEXEC cpack.exe CPACK_PATH "%CPACK_PATH%"
call :FINDEXEC git.exe GIT_PATH "%GIT_PATH%"

echo Checking for root source path [%SOURCE_DIR%]...
if NOT EXIST "%SOURCE_DIR%\src" echo Root path for source is not valid. & GOTO :EOF
if NOT EXIST "%SOURCE_DIR%\BUILD_NUMBER" set VERSION_FILE=VERSION-DIST

call :ABSPATH "%SOURCE_DIR%\" SRC_DIR
set CCI_VERSION_SRC_LIST=%SRC_DIR%BUILD_NUMBER %SRC_DIR%cci %SRC_DIR%cmake %SRC_DIR%CMakeLists.txt %SRC_DIR%external %SRC_DIR%include %SRC_DIR%src^/base %SRC_DIR%src^/cci %SRC_DIR%win^/

echo Checking build number with [%SOURCE_DIR%\%VERSION_FILE%]...
for /f %%i IN (%SOURCE_DIR%\%VERSION_FILE%) DO set VERSION=%%i
if ERRORLEVEL 1 echo Cannot check build number. & GOTO :EOF
for /f "tokens=1,2,3,4 delims=." %%a IN (%SOURCE_DIR%\%VERSION_FILE%) DO (
  set MAJOR_VERSION=%%a
  set MINOR_VERSION=%%b
  set PATCH_VERSION=%%c
  set EXTRA_VERSION=%%d
)
if NOT "%EXTRA_VERSION%." == "." (
  for /f "tokens=1,* delims=-" %%a IN ("%EXTRA_VERSION%") DO set SERIAL_NUMBER=%%a
) else (
  if EXIST "%SOURCE_DIR%\.git" (
    for /f "delims=" %%i in ('"%GIT_PATH%" rev-list --count --after %CCI_VERSION_START_DATE% HEAD %CCI_VERSION_SRC_LIST%') do set SERIAL_NUMBER=0000%%i
    for /f "delims=" %%i in ('"%GIT_PATH%" rev-parse HEAD') do set HASH_TAG=%%i
  ) else (
    set EXTRA_VERSION=0000
    set SERIAL_NUMBER=0000
  )
)
set SERIAL_NUMBER=%SERIAL_NUMBER:~-4%

if NOT "%HASH_TAG%." == "." set HASH_TAG=%HASH_TAG:~0,7%

if NOT "%HASH_TAG%." == "." set EXTRA_VERSION=%SERIAL_NUMBER%-%HASH_TAG%

echo Build Version is [%VERSION% (%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%)]
set VERSION=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%
set BUILD_NUMBER=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%EXTRA_VERSION%

if "%BUILD_TARGET%" == "Win32" (set CUBRID_CCI_PACKAGE_NAME=CUBRID-CCI-Windows-x86-%VERSION%) ELSE set CUBRID_CCI_PACKAGE_NAME=CUBRID-CCI-Windows-x64-%VERSION%

set BUILD_ROOT_DIR=%SOURCE_DIR%\Build_Win
set BUILD_DIR=%BUILD_ROOT_DIR%\build_%BUILD_MODE%_%BUILD_TARGET%_%BUILD_GEN_VERSION%
if NOT EXIST "%BUILD_ROOT_DIR%" md %BUILD_ROOT_DIR%
if NOT EXIST "%BUILD_DIR%" md %BUILD_DIR%

rem TODO move build_prefix
set BUILD_ROOT_PREFIX=%SOURCE_DIR%\win\output
set BUILD_PREFIX=%BUILD_ROOT_PREFIX%\CUBRID_%BUILD_MODE%_%BUILD_TARGET%_%BUILD_GEN_VERSION%
echo Build install directory is [%BUILD_PREFIX%].

if "%DIST_DIR%." == "." set DIST_DIR=%BUILD_DIR%\output
call :ABSPATH "%DIST_DIR%" DIST_DIR
echo Packages Output directory is [%DIST_DIR%].
if NOT EXIST "%DIST_DIR%" md %DIST_DIR%
GOTO :EOF


:BUILD_CUBRID
echo Building CUBRID in %BUILD_DIR%
cd /d %BUILD_DIR%

rem TODO: get generator from command line
if "%BUILD_TARGET%" == "Win32" (
  set CMAKE_GENERATOR=%BUILD_GENERATOR%
) else (
  set CMAKE_GENERATOR="%BUILD_GENERATOR:"=% Win64"
)
echo CMAKE_GENERATOR is [%CMAKE_GENERATOR%]

if "%BUILD_GEN_VERSION%" == "V141" (
  "%CMAKE_PATH%" -G %CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=%BUILD_PREFIX% -DPARALLEL_JOBS=10 -DCUBRID_CCI_PACKAGE_NAME=%CUBRID_CCI_PACKAGE_NAME% %SOURCE_DIR%
) else (
  "%CMAKE_PATH%" -G %CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=%BUILD_PREFIX% -DPARALLEL_JOBS=10 -DCUBRID_CCI_PACKAGE_NAME=%CUBRID_CCI_PACKAGE_NAME% -DFOR_OTHER_DRIVER=true %SOURCE_DIR%
)

if ERRORLEVEL 1 (echo FAILD. & GOTO :EOF) ELSE echo OK.

"%CMAKE_PATH%" --build . --config %BUILD_TYPE% --target install
if ERRORLEVEL 1 (echo FAILD. & GOTO :EOF) ELSE echo OK.

GOTO :EOF


:BUILD_CCI_PACKAGE
echo Buiding CCI package in %BUILD_DIR% ...
if NOT EXIST %BUILD_DIR% echo Cannot found built directory. & GOTO :EOF
cd /d %BUILD_DIR%

echo drop %CUBRID_CCI_PACKAGE_NAME%.zip into %DIST_DIR%
"%CPACK_PATH%" -C %BUILD_TYPE% -G ZIP -B "%DIST_DIR%"
if ERRORLEVEL 1 echo FAILD. & GOTO :EOF
rmdir /s /q "%DIST_DIR%"\_CPack_Packages
echo Package created. [%DIST_DIR%\%CUBRID_CCI_PACKAGE_NAME%.zip]
set DIST_PKGS=%DIST_PKGS% %CUBRID_CCI_PACKAGE_NAME%.zip
GOTO :EOF

:ABSPATH
set %2=%~f1
GOTO :EOF

:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]
GOTO :EOF


:BUILD_CLEAN
if "%OPTION_CHECK%" == "false" (
del /Q /S %BUILD_ROOT_DIR% >nul 2>&1
rmdir /s /q %BUILD_ROOT_DIR% >nul 2>&1
del /Q /S %BUILD_ROOT_PREFIX% >nul 2>&1
rmdir /s /q %BUILD_ROOT_PREFIX% >nul 2>&1
if EXIST %SOURCE_DIR%\CCI-VERSION-DIST del /Q /S %SOURCE_DIR%\CCI-VERSION-DIST >nul 2>&1
) else (
del /Q /S %BUILD_DIR% >nul 2>&1
rmdir /s /q %BUILD_DIR% >nul 2>&1
del /Q /S %BUILD_PREFIX% >nul 2>&1
rmdir /s /q %BUILD_PREFIX% >nul 2>&1
if EXIST %SOURCE_DIR%\CCI-VERSION-DIST del /Q /S %SOURCE_DIR%\CCI-VERSION-DIST >nul 2>&1
)
GOTO :EOF


:SHOW_USAGE
@echo.Usage: build.bat [OPTION] [TARGET]
@echo.Build and package script for CUBRID CCI (with tools - cci_applier)
@echo. OPTIONS
@echo.  /32      or /64    Build 32bit or 64bit applications (default: 64)
@echo.  /Release or /Debug Build with release or debug mode (default: Release)
@echo.  /vs2017            Build with VS2017 (default: VS2017)
@echo.  /vs2015 or /vs2012 Build with VS2015/2012
@echo.  /help /h /?        Display this help message and exit
@echo.
@echo. TARGETS
@echo.  all                Build and Packaging
@echo.  build              Build (default)
@echo.
@echo. Examples:
@echo.  build.bat                        # Build and pack CCI packages with default option
@echo.  build.bat clean                  # Clean
@echo.  build.bat /32 build              # 32bit release build only
@echo.  build.bat /64 /debug all         # 64bit debug mode Build and pack CCI packages
@echo.  build.bat /vs2012 /64 /debug all # 64bit debug mode Build and pack CCI packages with vs2012 generator
GOTO :EOF



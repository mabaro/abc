@echo off

SET BUILD_DIR=build

REM -DCMAKE_BUILD_TYPE=Debug,Release
SET BUILD_MODE=Release
SET DEFINES=-DENABLE_UNIT_TESTS=ON -DENABLE_EXCEPTIONS=ON -DENABLE_RTTI=ON

pushd

if "%~1" == "run" goto RUN
if "%~1" == "clean" goto CLEAN
if "%~1" == "build" goto BUILD
if "%~1" == "generate" goto GENERATE
if "%~1" == "test" goto TEST
if "%~1" == "deps" goto DEPS
if EXIST %BUILD_DIR% goto BUILD
goto GENERATE

:CLEAN
echo Cleaning project files...
rm -rf %BUILD_DIR%
goto END

:GENERATE

echo Generating project files...

mkdir %BUILD_DIR%

REM generate project + perform build
cmake -S . -B %BUILD_DIR% %2 %3 %4 -DCMAKE_INSTALL_PREFIX=./_install %DEFINES%
REM cmake --configure  -G "Visual Studio 16 Win64" -DENABLE_UNIT_TESTS=ON -DENABLE_EXCEPTIONS=ON -DENABLE_RTTI=ON
REM cmake --build .

REM Configure (i.e., add -GNinja)
REM cmake -S . -B %BUILD_DIR%

REM build
REM cmake --build %BUILD_DIR%

REM test
REM cmake --build %BUILD_DIR% --target test

REM docs
REM cmake --build %BUILD_DIR% --target docs

REM using IDE
REM cmake -S . -B xbuild -GXcode
REM cmake --open xbuild
goto END

:BUILD
echo Building project...
set CMD=cmake --build %BUILD_DIR% --config %BUILD_MODE% %2 %3 %4
set TIME0=%TIME%
REM powershell "Measure-Command{ %CMD% | Out-Default}"
%CMD%
set TIME1=%TIME%
echo Elapsed time:
echo   %TIME1%
echo - %TIME0%
goto END

:DEPS
set OUTPUT_FILE=graph/dependencies.dot
echo Generating dependencies @%OUTPUT_FILE%
cmake . --build %BUILD_DIR% --graphviz=graph/%OUTPUT_FILE%
goto END

:DOCS
cmake --build %BUILD_DIR% --target docs
goto END

:RUN
echo Running artifact...
set TARGET=test1_run
if NOT "%~2" == "" (set TARGET="%~2")

echo cmake --build %BUILD_DIR% --target %TARGET% --config %BUILD_MODE%
cmake --build %BUILD_DIR% --target %TARGET% --config %BUILD_MODE%
goto END

:TEST
echo Running tests...
cmake --build %BUILD_DIR% --target RUN_TESTS --config %BUILD_MODE%
goto END

:END
popd

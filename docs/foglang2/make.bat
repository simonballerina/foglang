@echo off

REM --- default target ---
set TARGET=%1

REM If no argument passed, use default install location
if "%TARGET%"=="" (
    set TARGET=%LOCALAPPDATA%\Programs\foglang2\foglang2.exe
)

echo Building to %TARGET%

REM Ensure output directory exists
for %%I in ("%TARGET%") do set OUTDIR=%%~dpI
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

REM Compile
gcc -o "%TARGET%" main.c -lm

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build succeeded!

exit /b 0
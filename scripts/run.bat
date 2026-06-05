@echo off
setlocal

:: Path to the compiled executable (Debug build)
set EXE=..\x64\Debug\Project.exe

:: Path to images folder
set IMAGES=..\images

if not exist "%EXE%" (
    echo ERROR: Executable not found at %EXE%
    echo Build the project in Visual Studio first ^(Debug x64^).
    pause
    exit /b 1
)

if not exist "..\output" mkdir "..\output"

echo Running parking lot detector on all images...
echo,

for %%f in (%IMAGES%\*.jpg %IMAGES%\*.png) do (
    echo Processing: %%f
    "%EXE%" "%%f"
    echo,
)

echo Done. Results saved to ..\output\
pause

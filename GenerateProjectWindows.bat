@echo off
:: Create folder if not exists
if not exist vendor\bins\premake mkdir vendor\bins\premake
:: Move to folder
cd vendor\bins\premake
:: Check if premake5.exe exists, if not, download and extract it
if not exist premake5.exe (
    echo Since this is a first time setup, this setup file will automatically download and install Premake locally from GitHub. If you do not wish to proceed, you may exit safely now.
    pause
    echo Installing Premake...
    curl -L --output premake.zip --url https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip
    :: Extract only premake exe
    tar -xf premake.zip premake5.exe
    :: Delete the downloaded zip
    del -q premake.zip
) else (
    echo Premake5 is already installed.
)
:: Move back out to main directory
cd ../../../

:: Install dependencies
echo Installing dependencies.
call InstallDependencies.bat

:: Ask the user to select the IDE
echo Select the IDE you want to generate project files for:
echo.
echo   [1] Visual Studio 2022      [5] Xcode
echo   [2] Visual Studio 2019      [6] gmake
echo   [4] Code::Blocks            [7] gmake2
echo.
set /p IDEChoice="Enter the number of your choice: "

:: Set the corresponding premake option
if "%IDEChoice%"=="1" set IDE=vs2022
if "%IDEChoice%"=="2" set IDE=vs2019
if "%IDEChoice%"=="4" set IDE=codeblocks
if "%IDEChoice%"=="5" set IDE=xcode
if "%IDEChoice%"=="6" set IDE=gmake
if "%IDEChoice%"=="7" set IDE=gmake2

:: Generate project files with the selected IDE
if defined IDE (
    vendor\bins\premake\premake5.exe --file=GenerateProjectFiles.lua %IDE%
    pause
) else (
    echo Invalid selection. Exiting...
    pause
)
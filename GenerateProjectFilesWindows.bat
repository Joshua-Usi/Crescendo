@echo off
set VULKAN_VERSION=1.3.283.0

:: Check if Vulkan SDK is already installed
if not exist C:\VulkanSDK\%VULKAN_VERSION% (
    echo Since this is a first time setup, this setup file will automatically download and install the Vulkan SDK and the necessary components from LunarG. If you do not wish to proceed, you may exit safely now.
    echo.
    echo This first time setup also requires administrator privileges to install the Vulkan SDK
    echo.
    echo Once a Vulkan SDK has been installed, you can run this script without administrator privileges
    pause

    echo Downloading Vulkan SDK version %VULKAN_VERSION%...
    curl -L --output VulkanSDK.exe --url https://sdk.lunarg.com/sdk/download/%VULKAN_VERSION%/windows/vulkan-sdk.exe

    echo Installing Vulkan SDK version %VULKAN_VERSION% with Volk and Vma components...
    VulkanSDK.exe --root C:\VulkanSDK\%VULKAN_VERSION% in com.lunarg.vulkan.vma in com.lunarg.vulkan.volk

    echo Setting up environment variables...
    setx VULKAN_SDK "C:\VulkanSDK\%VULKAN_VERSION%"
    setx PATH "%VULKAN_SDK%\Bin;%PATH%"

    :: Clean up the installer
    del /q VulkanSDK.exe
) else (
    echo Vulkan SDK version %VULKAN_VERSION% is already installed.
)

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

:: Ask the user for the project name
set /p projectName="Enter the project name: "

:: Check if GenerateProjectFiles.base exists and copy it to GenerateProjectFiles.lua
if exist "GenerateProjectFiles.base" (
    copy /Y "GenerateProjectFiles.base" "GenerateProjectFiles.lua"
    :: Replace <project_name> with the user-provided project name
    powershell -Command "(Get-Content 'GenerateProjectFiles.lua') -replace '<project_name>', '%projectName%' | Set-Content 'GenerateProjectFiles.lua'"
    powershell -Command "(Get-Content 'GenerateProjectFiles.lua') -replace '<vulkan_ver>', '%VULKAN_VERSION%' | Set-Content 'GenerateProjectFiles.lua'"
    echo Created GenerateProjectFiles.lua
) else (
    echo Error: GenerateProjectFiles.base is missing
    pause
    exit /b
)

:: Ask the user to select the IDE
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
    :: Clean up the project file lua
    del /q GenerateProjectFiles.lua
    pause
) else (
    echo Invalid selection. Exiting...
    pause
)
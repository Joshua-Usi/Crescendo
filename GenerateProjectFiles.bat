@echo off
echo Note! you will need to download or have downloaded the VulkanSDK
echo Generating Project Files...
:: Create folder if not exists
if not exist "vendor\bins\premake" mkdir "vendor\bins\premake"
:: Move to folder
cd vendor\bins\premake
:: Download premake
curl -s -L --output premake.zip --url https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip
:: Extract only premake exe
tar -xf premake.zip premake5.exe
:: Delete the downloaded zip
del -q premake.zip
:: Move back out to main directory
cd ../../../
:: Generate project files
vendor\bins\premake\premake5.exe --file=GenerateProjectFiles.lua vs2022
echo Done
pause
@echo off

echo Downloading dependencies
git clone https://github.com/Joshua-Usi/CrescendoThirdParty ./Crescendo/ThirdParty

git clone https://github.com/Joshua-Usi/Mochi ./Tests/Mochi

echo Updating dependencies
git submodule update --recursive --remote

vendor\bins\premake\premake5.exe --file=GenerateProjectFiles.lua vs2022
pause
@echo off

echo Downloading dependencies
git submodule add https://github.com/Joshua-Usi/CrescendoThirdParty ./Crescendo/ThirdParty
git submodule add https://github.com/Joshua-Usi/Mochi ./Tests/Mochi

echo Updating dependencies
git submodule update --recursive --remote
pause
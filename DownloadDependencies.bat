@echo off

echo Downloading dependencies
git submodule add https://github.com/Joshua-Usi/CrescendoThirdParty ./Crescendo/ThirdParty
if errorlevel 1 goto Update
pause

:Update
echo Dependencies already exist
echo Updating dependencies
git submodule update --recursive --remote
pause
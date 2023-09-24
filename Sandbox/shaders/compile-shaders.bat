@echo off
set "source_path=%~dp0"
set "dest_path=%source_path%\compiled"
set "compiler_path=C:/VulkanSDK/1.3.243.0/Bin/glslc.exe"

rem Create compiled subdirectory if it doesn't exist
if not exist "%dest_path%" mkdir "%dest_path%"

rem Loop over all .vert and .frag files
for /R "%source_path%" %%F in (*.vert, *.frag) do (
    rem Get the file name with extension
    for /F "delims=" %%G in ("%%~nxF") do (
        rem Compile and output to the compiled subdirectory with .spv extension
        "%compiler_path%" "%%F" -o "%dest_path%\%%G.spv"
    )
)

echo Shaders Compiled!
pause
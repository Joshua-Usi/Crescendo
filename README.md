# Crescendo
> Game Engine using the Vulkan API

![A screenshot of the current state of the engine](/Crescendo.jpg)

# Setting up a project
Crescendo aims to make it as easy as possible to set up a project, choose the relevant section for your platform for more information

## Windows
1. run `GenerateProjectWindows.bat`, __NOTE!__ that on first time setup, you need to run this with __administrator privileges__ as it will install the required Vulkan SDK for you. If you do not wish to run with administrator privileges, you can preinstall Vulkan SDK 1.3.283 with the VMA and Volk components turned on. All subsequent runs of this setup file can be run without administrator privileges.

2. Select a project name

3. Select the IDE you want, Crescendo best supports Visual Studio

| Option | IDE                 | Option | IDE                 |
|--------|---------------------|--------|---------------------|
| [1]    | Visual Studio 2022  | [4]    | Xcode               |
| [2]    | Visual Studio 2019  | [5]    | gmake               |
| [3]    | Code::Blocks        | [6]    | gmake2              |

If you chose Visual Studio, a solution file with the project name will be created in the same directory, Open this solution file to begin development


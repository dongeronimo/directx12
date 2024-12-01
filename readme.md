## MyDirectX studies

## Setup
1) Install windows sdk
2) clone
3) update submodules: git submodule update --init --recursive
4) open assimp project with cmake, set output directory to ./assimp-build
5) Add entry ASSIMP_BUILD_FBX_IMPORTER = true, ASSIMP_BUILD_DRACO = true ASSIMP_BUILD_GLTF_IMPORTER = true and disable build all importers
and build all exporters.
    - or disable build all exporters and enable build all importers if you don't care about a bigger lib
6) open assimp project in visual studio, run ALL_BUILD, close, reopen as admin, run INSTALL  
7) open directx-headers with cmake, set output directory to DirectX-Headers\Build
8) generate directx-headers project
9) open directx-headers project, run ALL_BUILD, close, then open VS again as elevated user, and run INSTALL
10) open MyDirectx12.sln visual studio 2024
11) build debug but don't run, because the assets won't be available
12) run symbolic_links.bat as admin
13) now you can run

Remember that if you want a build release you have to build assimp and directxheaders as release because they need the .libs
Also, remember to copy the assimp dlls from it's install folder in program files to the same folder that the .exes are.

Troubleshooting:
- Fail to load shaders: the .cso, by default go to $(SolutionDir)$(Platform)\$(Configuration). Make sure that the project's working directory is $(SolutionDir)$(Platform)\$(Configuration).
- Fail to build release: compile assimp and directxmath as release.
- Crashes due to lack of assimp dll: copy the dll from assimp install dir (the one in program files) to the same dir that the executable is.

## Projects
- Common: code that's shared between the projects
- HelloWorld: first triangle. how to setup a window, create the directx infrastructure and put something on the screen
- ColoredTriangle: triangle with color. How to pass data to the shaders, in this example, position and color. 

## Submodules
- DirectX-Headers (https://github.com/microsoft/DirectX-Headers.git)
- assimp (https://github.com/assimp/assimp.git)

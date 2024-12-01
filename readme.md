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
10) run symbolic_links.bat as admin
11) open MyDirectx12.sln visual studio 2024
12) build

## Projects
- Common: code that's shared between the projects
- HelloWorld: first triangle. how to setup a window, create the directx infrastructure and put something on the screen
- ColoredTriangle: triangle with color. How to pass data to the shaders, in this example, position and color. 

## Submodules
- DirectX-Headers (https://github.com/microsoft/DirectX-Headers.git)
- assimp (https://github.com/assimp/assimp.git)

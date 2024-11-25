// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <initguid.h> //include <initguid.h> before including d3d12.h, and then the IIDs will be defined instead of just declared. 
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <cassert>
#include <vector>
#include <array>
#include <DirectXMath.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <wrl/client.h>
#include <functional>
#include <optional>

#endif //PCH_H

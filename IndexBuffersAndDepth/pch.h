#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
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
#include <memory>
#include <map>
#include <unordered_map>
#include <filesystem>
constexpr int FRAMEBUFFER_COUNT = 2;
constexpr bool FULLSCREEN = false;

struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, z) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};
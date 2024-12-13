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
#include <memory>
#include <DirectXMath.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <wrl/client.h>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <type_traits>
#include <random>
namespace common
{
	class Mesh;
}
namespace common::images
{
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateImage(int textureWidth, 
		int textureHeight, 
		DXGI_FORMAT format,
		UINT sampleCount, 
		UINT sampleQuality, 
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		std::array<float, 4> cv = {0,0,0,0});

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthImage(int textureWidth,
		int textureHeight,
		UINT sampleCount,
		UINT sampleQuality,
		Microsoft::WRL::ComPtr<ID3D12Device> device);

}

namespace common::io
{
	/// <summary>
	/// remember that a file can have many meshes, that's why is a vector
	/// </summary>
	/// <param name="device"></param>
	/// <param name="queue"></param>
	/// <param name="filepathInAssetFolder"></param>
	/// <returns></returns>
	std::vector<std::shared_ptr<common::Mesh>> LoadMesh(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue,
		std::string filepathInAssetFolder);
}


#endif //PCH_H

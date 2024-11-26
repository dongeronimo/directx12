#pragma once
#include "pch.h"
namespace myd3d
{
	struct MeshData
	{
		std::string name;
		std::vector<uint16_t> indices;
		std::vector<DirectX::XMFLOAT3> vertices;
		std::vector<DirectX::XMFLOAT3> normals;
		std::vector<DirectX::XMFLOAT2> uv;
	};

	std::vector<myd3d::MeshData> LoadMeshes(
		const std::string& filename
	);
}


#pragma once
#include "pch.h"
namespace rtt::entities
{
	struct Transform
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 scale;
		DirectX::XMVECTOR rotation;
	};

	struct MeshRenderer
	{
		uint32_t idx;
	};

	struct GameObject
	{
		std::wstring name;
	};
}
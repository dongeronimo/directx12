#pragma once
#include "pch.h"

namespace transforms
{
	class Transform
	{
	public:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 scale;
		DirectX::XMVECTOR rotation;
		int id;
		virtual ~Transform() = default;
	};
}
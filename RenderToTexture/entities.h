#pragma once
#include "pch.h"
namespace rtt::entities
{
	/// <summary>
	/// Position, scale and rotation. 
	/// There's no rotation around pivot at the moment
	/// </summary>
	struct Transform
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 scale;
		DirectX::XMVECTOR rotation;
	};
	/// <summary>
	/// Variation of transform over time.
	/// There's no rotation around pivot yet
	/// </summary>
	struct DeltaTransform
	{
		DirectX::XMFLOAT3 dPosition;
		DirectX::XMFLOAT3 dScale;
		DirectX::XMVECTOR rotationAxis;
		float rotationSpeed;
	};
	/// <summary>
	/// The Id of the mesh. The mesh's somewhere else
	/// </summary>
	struct MeshRenderer
	{
		uint32_t idx;
	};
	/// <summary>
	/// For now is just the name.
	/// </summary>
	struct GameObject
	{
		std::wstring name;
	};
	
	struct Cube
	{
		uint32_t idx;
	};

	struct Monkey
	{
		uint32_t idx;
	};
}
#pragma once
#include "pch.h"
namespace common //TODO refactor: change namespace to commons
{
	/// <summary>
	/// Standard vertex with pos, normal and uv
	/// </summary>
	struct Vertex {
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v) : 
			pos(px, py, pz), normal(nx,ny, nz), uv(u,v) {}
		Vertex() {}
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
}
#pragma once
#include "pch.h"
namespace dx3d
{
	namespace input_layout_service
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> OnlyVertexes();
		std::vector<D3D12_INPUT_ELEMENT_DESC> PositionsAndColors();
		std::vector<D3D12_INPUT_ELEMENT_DESC> PositionsNormalsAndUVs();
	}
}


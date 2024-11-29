#include "pch.h"
#include "input_layout_service.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> common::input_layout_service::OnlyVertexes()
{
    std::vector< D3D12_INPUT_ELEMENT_DESC> inputLayout(
        {
            {
                "POSITION", //goes into POSITION in the shader
                0, //index 0 in that semantic.  
                DXGI_FORMAT_R32G32B32_FLOAT,  //x,y,z of 32 bits floats 
                0, //only binding one buffer at a time 
                0, //atribute offset, this is the first and only attribute so 0
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //vertex data
                0
            }
        });
    return inputLayout;
}

std::vector<D3D12_INPUT_ELEMENT_DESC> common::input_layout_service::PositionsAndColors()
{

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    return inputLayout;
}

std::vector<D3D12_INPUT_ELEMENT_DESC> common::input_layout_service::PositionsNormalsAndUVs()
{
    constexpr size_t vertexSize = sizeof(float) * 3;
    constexpr size_t vertexOffset = 0;
    constexpr size_t normalsOffset = vertexSize;
    constexpr size_t normalsSize = sizeof(float) * 3;
    constexpr size_t uvSize = sizeof(float) * 2;
    constexpr size_t uvOffset = vertexSize + normalsSize;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, vertexOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalsOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uvOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    return inputLayout;
}

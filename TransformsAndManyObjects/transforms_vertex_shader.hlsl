//inputs
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};
//outputs
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};
//describes the model matrix
struct ModelMatrixStruct
{
    float4x4 mat;
};
//where i store the model matrices
StructuredBuffer<ModelMatrixStruct> ModelMatrices : register(t0);
//a root constant (aka push constant), it's the index at ModelMatrices 
cbuffer RootConstants : register(b0)
{
    uint matrixIndex; // Root constant passed by CPU
}
//holds the view projection matrix
cbuffer ConstantBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4x4 modelMatrix = ModelMatrices[matrixIndex].mat;
    float4 worldPosition = mul(float4(input.pos, 1.0f), modelMatrix);
    output.pos = mul(worldPosition, viewProjectionMatrix);
    output.color = float4(input.uv, 1, 1);
    return output;
}
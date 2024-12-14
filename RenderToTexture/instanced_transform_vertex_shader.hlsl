//inputs
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    int instanceID : OBJECT_ID; // Instance ID semantic
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

//holds the view projection matrix
cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProjectionMatrix;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4x4 modelMatrix = ModelMatrices[input.instanceID].mat;
    float4x4 mvpMatrix = mul(modelMatrix, viewProjectionMatrix);
    output.pos = mul(float4(input.pos, 1.0f), mvpMatrix);
    output.color = float4(input.uv, 1.0f, 1.0f);
    return output;
}
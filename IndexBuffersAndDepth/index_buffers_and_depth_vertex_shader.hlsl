struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProjectionMatrix;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), viewProjectionMatrix);
    output.color = float4(input.uv,1, 1);
    return output;
}
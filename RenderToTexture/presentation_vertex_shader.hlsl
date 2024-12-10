struct VertexInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL; //not used but the vertex data comes with it, so i declare it here
    float2 uv : UV;
};

struct VertexOutput
{
    float4 position : SV_POSITION; // Clip-space position (output to rasterizer)
    float2 uv : TEXCOORD; // Pass texture coordinates to pixel shader
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.pos, 1.0f); // Pass position to rasterizer
    output.uv = input.uv; // Pass UV coordinates to pixel shader
    return output;
}

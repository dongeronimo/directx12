struct VertexInput
{
    float4 position : POSITION; // Clip-space position
    float2 uv : TEXCOORD; // Texture coordinates
};

struct VertexOutput
{
    float4 position : SV_POSITION; // Clip-space position (output to rasterizer)
    float2 uv : TEXCOORD; // Pass texture coordinates to pixel shader
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = input.position; // Pass position to rasterizer
    output.uv = input.uv; // Pass UV coordinates to pixel shader
    return output;
}

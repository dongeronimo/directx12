Texture2D texture0 : register(t0); // Texture bound to slot t0
SamplerState sampler0 : register(s0); // Sampler bound to slot s0

struct VertexOutput
{
    float4 position : SV_POSITION; // Clip-space position
    float2 uv : TEXCOORD; // Texture coordinates
};

float4 main(VertexOutput input) : SV_TARGET
{
    return texture0.Sample(sampler0, input.uv); // Sample the texture
}

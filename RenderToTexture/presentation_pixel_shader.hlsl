Texture2D texture0 : register(t0); // Texture bound to slot t0
SamplerState sampler0 : register(s0); // Sampler bound to slot s0

struct VertexOutput
{
    float4 position : SV_POSITION; // Clip-space position
    float2 uv : TEXCOORD; // Texture coordinates
};

float4 main(VertexOutput input) : SV_TARGET
{
    //TODO: texel size varies with the size of the viewport, it must be a root parameter, probably a root constant.
    float2 texelSize; // Size of a single texel: float2(1.0 / textureWidth, 1.0 / textureHeight)
    texelSize.x = 1.0f / 800.f;
    texelSize.y = 1.0f / 600.f;
    //TODO: gaussian should come from a root parameter, both the size of the kernel and the values in it.
    // Gaussian kernel weights for a 5-sample blur
    float weights[5] = { 0.227027, 0.316216, 0.070270, 0.002216, 0.001027 };
    // horizontal blur, i'm lazy and that's enough to demosntrate post processing
    float4 color = float4(0, 0, 0, 0);
    for (int i = -2; i <= 2; i++)
    {
        float2 offset = float2(i * texelSize.x, 0.0); // Horizontal offset
        color += texture0.Sample(sampler0, input.uv + offset) * weights[abs(i)];
    }
    
    return color; 
}

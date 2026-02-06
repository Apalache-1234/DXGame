struct output
{
    float4 position : SV_POSITION;

    float2 TexCoord : TEXCOORD;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
    
    float index : INDEX;
};

struct pixelOp
{
    float4 color : SV_TARGET0;
};

cbuffer RadialBlurSettings : register(b3)
{
    float2 screenCenter : packoffset(c0);
    float blurStrength : packoffset(c1);
    int sampleCount : packoffset(c2);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1 - uv.y;
    
    float2 direction = uv - screenCenter;

    float4 color = float4(0, 0, 0, 0);
    float weightSum = 0.0;

    for (int i = 0; i < sampleCount; ++i)
    {
        float t = i / (float) (sampleCount - 1);
        float2 sampleUV = uv + direction * t * blurStrength;
        sampleUV = saturate(sampleUV);
        color += tex.Sample(tex_sampler, sampleUV);
        weightSum += 1.0;
    }
    
    color /= weightSum;
    
    output_pixel.color = color;
    
    return output_pixel;
}
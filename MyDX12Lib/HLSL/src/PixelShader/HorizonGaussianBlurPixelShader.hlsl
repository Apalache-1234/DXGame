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

cbuffer GaussianBlurInfo : register(b3)
{
    float sigma : packoffset(c0);
    float kernelRadius : packoffset(c1);
    float direction : packoffset(c2);
};

#include <Header.hlsli> 

float gaussian(float x, float sigma)
{
    return exp(-0.5 * (x * x) / (sigma * sigma));
}

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;
    
    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1 - uv.y;
    
    float4 color = float4(0, 0, 0, 0);
    float weightSum = 0.0f;

    float2 dir = float2(1, 0);
    
    for (int i = -kernelRadius; i <= kernelRadius; ++i)
    {
        float2 offset = dir * Materials[vertexOp.index].TEXELSIZE.xy * i;
        float weight = gaussian(i, sigma);
        
        float2 uvAdj = uv + offset;
        uvAdj.x = saturate(uvAdj.x);

        color += tex.Sample(tex_sampler, uvAdj) * weight;
        weightSum += weight;
    }

    output_pixel.color = color / weightSum;

    return output_pixel;
}
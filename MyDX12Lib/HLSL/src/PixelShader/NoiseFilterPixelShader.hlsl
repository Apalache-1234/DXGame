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

cbuffer NoiseFilterInfo : register(b3)
{
    float intensify : packoffset(c0);
    float blockSize : packoffset(c1);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;
    
    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1.0 - uv.y;
    
    if (random(floor(uv.y / 1.1234) * 1.1234 + RealTime) > 0.9)
    {
        uv.x += (0.5 - random(floor(uv.y / 0.001) * 0.001 + RealTime)) * intensify;
    }
    
    float4 color = tex.Sample(tex_sampler, uv);
    
    uv = floor(uv / blockSize) * blockSize;
    
    output_pixel.color.rgb = lerp(color.rgb, float3(random(uv + RealTime), random(uv + RealTime * 2.2), random(uv + RealTime * 3.6)), intensify);

    output_pixel.color.a = 1;
    
    return output_pixel;
}
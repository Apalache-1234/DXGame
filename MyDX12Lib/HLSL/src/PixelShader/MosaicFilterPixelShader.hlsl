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

cbuffer MosaicFilterInfo : register(b3)
{
    float intensify : packoffset(c0);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;
    
    float2 texelSize = Materials[vertexOp.index].TEXELSIZE.xy;
    float2 step = texelSize * intensify;

    int2 grid = (vertexOp.TexCoord - 0.5) / step;
    float2 uv = grid * step + 0.5;

    uv.y = 1.0 - uv.y;
    uv = clamp(uv, 0.0, 1.0);

    float4 color = tex.Sample(tex_sampler, uv);
    output_pixel.color = color;

    return output_pixel;
}
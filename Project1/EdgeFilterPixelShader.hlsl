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

cbuffer EdgeFilterInfo : register(b3)
{
    float transition : packoffset(c0);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1 - uv.y;
    
    float4 color = tex.Sample(tex_sampler, uv);
    
    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    float4 edgeColor = fwidth(luminance);
    
    edgeColor.a = 1;
        
    output_pixel.color = lerp(color, edgeColor, transition);
    
    return output_pixel;
}
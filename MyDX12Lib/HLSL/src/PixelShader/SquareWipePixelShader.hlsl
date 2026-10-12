struct output
{
    float4 position : SV_POSITION;

    float2 TexCoord : TEXCOORD;

    float4 color : COLOR0;
};

struct pixelOp
{
    float4 color : SV_TARGET0;
};

#include <Header.hlsli> 

cbuffer SquareInfo : register(b33)
{
    float4x4 rotate: packoffset(c0);
    float scale : packoffset(c4);
};

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
	
    uv.y = 1.0 - uv.y;
	
    output_pixel.color = tex.Sample(tex_sampler, uv) * vertexOp.color;
    
    output_pixel.color.a = 1;
    
    uv -= 0.5;
    
    uv = mul(rotate, float4(uv, 0, 0));
    
    if (abs(uv.x) < scale && abs(uv.y) < scale)
        discard;

    return output_pixel;
}
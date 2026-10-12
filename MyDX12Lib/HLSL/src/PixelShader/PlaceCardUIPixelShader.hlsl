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

cbuffer CardInfo : register(b35)
{
    float time : packoffset(c0);
    float4 uiColor : packoffset(c1);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
	
    uv.y = 1.0 - uv.y;
    
    uv.x = uv.x * 20;
    
    uv = frac(uv);
    
    if (uv.x < 0.5)
    {
        uv.y += pow(1 - (time % 2) * 2, 2);
    }
    else
    {
        uv.y -= pow(1 - (time % 2) * 2, 2);
    }
    
    if (uv.y < 0 || uv.y > 1)
        output_pixel.color = uiColor;
    else if (uv.y < 0.3 || uv.y > 0.7)
        output_pixel.color = float4(0, 0, 0, 1);
    else
        output_pixel.color = uiColor;

   
    output_pixel.color *= color;
    
    return output_pixel;
}
struct output
{
    float4 position : SV_POSITION;

    float2 TexCoord : TEXCOORD;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
};

struct pixelOp
{
    float4 color : SV_TARGET0;
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
    
    uv.y = -uv.y;
    
    output_pixel.color = tex.Sample(tex_sampler, uv);
    
    float luminance = dot(output_pixel.color.rgb, float3(0.299, 0.587, 0.114));
    
    output_pixel.color = ddx(luminance) + ddy(luminance);
    
    output_pixel.color.a = 1;

    return output_pixel;
}
struct output
{
    float4 position : SV_POSITION;

    float4 worldPos : POSITION;

    float2 TexCoord : TEXCOORD;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
	
    nointerpolation float index : INDEX;
};

struct pixelOp
{
	float4 color: SV_TARGET0;
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1 - uv.y;

	output_pixel.color = tex.Sample(tex_sampler , uv) * vertexOp.color;

    if (output_pixel.color.a < 0.9)
        output_pixel.color.rgba = 1;
    
    vertexOp.normal = ApplyNormalMap(uv, vertexOp.normal, vertexOp.tangent);

	output_pixel.color = ApplyLightShading(output_pixel.color, vertexOp.worldPos ,vertexOp.normal , vertexOp.index);
    
	return output_pixel;
}
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
	float4 color: SV_TARGET0;
};

#include <Header.hlsli> 

Texture2D tex : register(t0);

SamplerState tex_sampler : register(s0);

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

	float2 uv = vertexOp.TexCoord;

	output_pixel.color = tex.Sample(tex_sampler , uv ) * vertexOp.color;

	output_pixel.color = ApplyLightShading(output_pixel.color, vertexOp.normal);

	if (output_pixel.color.a == 0.0f) discard;
	
	

	return output_pixel;
}
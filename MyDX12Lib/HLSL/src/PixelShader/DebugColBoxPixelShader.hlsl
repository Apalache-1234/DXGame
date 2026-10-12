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

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

	float2 len = abs(vertexOp.TexCoord % 1 - 0.5);

	float3 nor = normalize(vertexOp.normal);

	output_pixel.color = vertexOp.color;

	if (output_pixel.color.a == 0.0f || (len.x < 0.499 && len.y < 0.499)) discard;

	return output_pixel;
}
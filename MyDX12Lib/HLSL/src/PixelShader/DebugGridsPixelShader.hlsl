struct output
{
	float4 position : SV_POSITION;

	float3 worldPos : POSITION;

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

	float2 toCam = (vertexOp.worldPos - cameraPos).xz;

	output_pixel.color = vertexOp.color *  pow(1 - (length(toCam) / 100), 1);

	return output_pixel;
}
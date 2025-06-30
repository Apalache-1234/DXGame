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

	float3 toCam = normalize(cameraPos - vertexOp.worldPos);

	output_pixel.color = vertexOp.color *  (1 - dot(toCam , normalize(vertexOp.normal)));

	if (output_pixel.color.a == 0.0f || dot(toCam, normalize(vertexOp.normal)) < 0 || dot(toCam, normalize(vertexOp.normal)) > 0.1) discard;

	return output_pixel;
}
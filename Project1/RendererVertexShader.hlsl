struct input
{
	float3 position : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 normal : NORMAL;

	float4 color : COLOR;

	float3 tangent : TANGENT;

	float4 boneID  : BONEID;

	float4 weight  : WEIGHT;

	float parent : PARENT;
};

struct output
{
	float4 position : SV_POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 color : COLOR0;
};

#include <Header.hlsli> 

output main(input vertexIn)
{
	output vertexOut = (output)0;

	float4 proj = TransLocalToProj(vertexIn.position);

	vertexOut.color = color;

	vertexOut.position = proj;

	vertexOut.TexCoord = vertexIn.TexCoord;

	vertexOut.tangent = vertexIn.tangent;

	vertexOut.normal = vertexIn.normal;

	return vertexOut;
}
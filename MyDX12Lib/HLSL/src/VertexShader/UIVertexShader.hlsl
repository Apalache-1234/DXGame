struct input
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct output
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR0;
};

#include <Header.hlsli> 

output main(input vertexIn)
{
	output vertexOut = (output)0;

	float4 proj = TransLocalToProj(vertexIn.position);

	vertexOut.position = proj;
	vertexOut.texCoord = vertexIn.texCoord;
	vertexOut.color = color;

	return vertexOut;
}
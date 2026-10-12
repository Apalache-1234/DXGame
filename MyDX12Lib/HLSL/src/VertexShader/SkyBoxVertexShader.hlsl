struct input
{
	float3 position : POSITION;
};

struct output
{
	float4 position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
    float4 color : COLOR0;
};

#include <Header.hlsli> 

output main(input vertexIn)
{
	output vertexOut = (output)0;

	float4 proj = TransLocalToProj(vertexIn.position);;

	vertexOut.position = proj;
	vertexOut.TexCoord = vertexIn.position;
    vertexOut.color = color;
	
	return vertexOut;
}
struct input
{
	float3 position : POSITION;
};

struct output
{
	float4 position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

#include <Header.hlsli> 

output main(input vertexIn)
{
	output vertexOut = (output)0;

	float4 local_pos = float4(vertexIn.position, 1.0f);

	float4 world = mul(world_mat, local_pos);

	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	vertexOut.position = proj;
	vertexOut.TexCoord = vertexIn.position;

	return vertexOut;
}
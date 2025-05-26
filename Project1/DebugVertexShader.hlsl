struct input
{
	float3 position : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 boneID  : BONEID;

	float4 weight  : WEIGHT;

	float parent : PARENT;

	float4 world0 : COLLIDER_WORLD0;
	float4 world1 : COLLIDER_WORLD1;
	float4 world2 : COLLIDER_WORLD2;
	float4 world3 : COLLIDER_WORLD3;

	float4 colors : COLORS;
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

	float4x4 worldMat = float4x4(vertexIn.world0, vertexIn.world1, vertexIn.world2, vertexIn.world3);

	float4 local_pos = float4(vertexIn.position, 1.0f);

	float4 world = mul(worldMat, local_pos);

	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	vertexOut.color = vertexIn.colors;

	vertexOut.position = proj;

	vertexOut.TexCoord = vertexIn.TexCoord;

	vertexOut.tangent = vertexIn.tangent;

	vertexOut.normal = vertexIn.normal;

	return vertexOut;
}
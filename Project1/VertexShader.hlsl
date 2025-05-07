struct input
{
	float3 position : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 boneID  : BONEID;

	float4 weight  : WEIGHT;
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

struct Bones
{
	float4x4 boneFinalMat;
};

StructuredBuffer<Bones> boneFinalMats : register(t1);

output main(input vertexIn)
{
	output vertexOut = (output)0;
	uint id0 = vertexIn.boneID.x; // ïâêîñhé~
		uint id1 = vertexIn.boneID.y;
		uint id2 = vertexIn.boneID.z;
		uint id3 = vertexIn.boneID.w;

float4x4 boneTransform =
    boneFinalMats[id0].boneFinalMat * vertexIn.weight.x +
    boneFinalMats[id1].boneFinalMat * vertexIn.weight.y +
    boneFinalMats[id2].boneFinalMat * vertexIn.weight.z +
    boneFinalMats[id3].boneFinalMat * vertexIn.weight.w;

	float4 local_pos;

	local_pos = mul(boneTransform, float4(vertexIn.position, 1.0f));

	if(length(local_pos) == 0)
	{
		local_pos = float4(vertexIn.position, 1.0f);
	}

	float4 world = mul(world_mat, local_pos);

	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	vertexOut.color = color;

	vertexOut.position = proj;

	vertexOut.TexCoord = vertexIn.TexCoord;

	vertexOut.tangent = vertexIn.tangent;

	vertexOut.normal = vertexIn.normal;

	return vertexOut;
}
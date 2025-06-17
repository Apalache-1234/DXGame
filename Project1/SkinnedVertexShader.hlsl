struct input
{
	float3 position : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 boneID  : BONEID;

	float4 weight  : WEIGHT;

	float parent  : PARENT;
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

StructuredBuffer<float4x4> boneFinalMats : register(t20);

output main(input vertexIn)
{
	output vertexOut = (output)0;

	int id0 = vertexIn.boneID.x ; // ïâêîñhé~
		int id1 = vertexIn.boneID.y;
		int id2 = vertexIn.boneID.z;
		int id3 = vertexIn.boneID.w;
		int id4 = vertexIn.parent;

float4x4 boneTransform =
    boneFinalMats[id0] * vertexIn.weight.x +
    boneFinalMats[id1] * vertexIn.weight.y +
    boneFinalMats[id2] * vertexIn.weight.z +
    boneFinalMats[id3] * vertexIn.weight.w;

	float4 local_pos;

	local_pos = mul(boneTransform, float4(vertexIn.position, 1.0f));

	if (length(local_pos) == 0)
	{
		local_pos = float4(vertexIn.position, 1.0f);
	}

	if (id4 != 0)
	{
		local_pos = mul(boneFinalMats[id4], local_pos);
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
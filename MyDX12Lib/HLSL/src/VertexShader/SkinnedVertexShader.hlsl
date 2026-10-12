struct input
{
	float3 position : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 boneID  : BONEID;

	float4 weight  : WEIGHT;

	float parent  : PARENT;
	
    float index : INDEX;
};

struct output
{
	float4 position : SV_POSITION;

	float4 worldPos : POSITION;

	float2 TexCoord : TEXCOORD;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 color : COLOR0;
	
    nointerpolation float index : INDEX;
};

#include <Header.hlsli> 

struct Bones
{
	float4x4 boneFinalMat;
};

cbuffer transform : register(b21)
{
    float4x4 node_trans_mat : packoffset(c0);
}

StructuredBuffer<float4x4> boneFinalMats : register(t20);

float4 ReturnAnimatedVerteces(input vertexIn)
{
	int id0 = vertexIn.boneID.x;
	int id1 = vertexIn.boneID.y;
	int id2 = vertexIn.boneID.z;
	int id3 = vertexIn.boneID.w;
	int id4 = vertexIn.parent;

	float4x4 boneTransform =
		boneFinalMats[id0] * vertexIn.weight.x +
		boneFinalMats[id1] * vertexIn.weight.y +
		boneFinalMats[id2] * vertexIn.weight.z +
		boneFinalMats[id3] * vertexIn.weight.w;

	float4 pos;
	
    pos = mul(boneTransform, float4(vertexIn.position, 1.0f));

	if (length(pos) == 0)
	{
        pos = float4(vertexIn.position, 1.0f);
    }

    pos = mul(node_trans_mat, pos);


	return pos;
}

float3 ReturnAnimatedNormal(float3 nor , input vertexIn , float4 vertexPos)
{
    int id0 = vertexIn.boneID.x;
    int id1 = vertexIn.boneID.y;
    int id2 = vertexIn.boneID.z;
    int id3 = vertexIn.boneID.w;
    int id4 = vertexIn.parent;

    float4x4 boneTransform =
		boneFinalMats[id0] * vertexIn.weight.x +
		boneFinalMats[id1] * vertexIn.weight.y +
		boneFinalMats[id2] * vertexIn.weight.z +
		boneFinalMats[id3] * vertexIn.weight.w;

    float4 pos;
	
	float3 norPos = nor + vertexIn.position;

    pos = mul(boneTransform, float4(norPos, 1));

    if (length(pos) == 0)
    {
        pos = float4(norPos, 1);
    }
	
    pos = mul(node_trans_mat, pos);

	
    float3 pos3 = (float3) (pos - vertexPos);

    return normalize(pos3);
}

output main(input vertexIn)
{
	output vertexOut = (output)0;

    float4 local = ReturnAnimatedVerteces(vertexIn);

	float4 proj = TransLocalToProj(local);

	vertexOut.color = color;

	vertexOut.position = proj;
	
    float4 world = mul(world_mat, local);
	
	vertexOut.worldPos = world;

    vertexOut.TexCoord = TransformUV(vertexIn.TexCoord , vertexIn.index);
	
	vertexOut.tangent = ReturnAnimatedNormal(vertexIn.tangent, vertexIn, local);
    vertexOut.tangent = normalize(mul(world_mat, vertexOut.tangent));
	
    vertexOut.normal = ReturnAnimatedNormal(vertexIn.normal, vertexIn , local);
    vertexOut.normal = normalize(mul(world_mat, vertexOut.normal));
	
    vertexOut.index = vertexIn.index;

	return vertexOut;
}
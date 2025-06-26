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

	if (id4 != 0)
	{
		pos = mul(boneFinalMats[id4], pos);
	}

	return pos;
}

float3 ReturnAnimatedNormal(input vertexIn)
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
	

    boneTransform._14 = 0;
    boneTransform._24 = 0;
    boneTransform._34 = 0;

    pos = mul(boneTransform, float4(vertexIn.normal, 1.0f));

    if (length(pos) == 0)
    {
        pos = float4(vertexIn.normal, 1.0f);
    }
	
    float3 pos3 = (float3) pos;

    return normalize(pos3);
}

output main(input vertexIn)
{
	output vertexOut = (output)0;

    float4 local = ReturnAnimatedVerteces(vertexIn);

	float4 world = mul(world_mat, local);
 
	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	vertexOut.color = color;

	vertexOut.position = proj;

	vertexOut.TexCoord = vertexIn.TexCoord;

	vertexOut.tangent = vertexIn.tangent; 
	
    vertexOut.normal = ReturnAnimatedNormal(vertexIn);

	return vertexOut;
}
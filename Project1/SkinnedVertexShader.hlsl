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

	float3 worldPos : POSITION;

	float2 TexCoord : TEXCOORD;
	
	float4 lightPos : LIGHT_POS;

	float3 tangent : TANGENT;

	float3 normal : NORMAL;

	float4 color : COLOR0;
	
    float index : INDEX;
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

	if (id4 != 0)
	{
		pos = mul(boneFinalMats[id4], pos);
	}
	
    float3 pos3 = (float3) (pos - vertexPos);

    return normalize(pos3);
}

output main(input vertexIn)
{
	output vertexOut = (output)0;

    float4 local = ReturnAnimatedVerteces(vertexIn);

	float4 proj = TransLocalToProj(local);
	
    float4 world = mul(world_mat, local);
	
    vertexOut.lightPos = mul(ambientLightVPMat, local);

	vertexOut.color = color;

	vertexOut.position = proj;

	vertexOut.worldPos = mul(world_mat , local);

    vertexOut.TexCoord = TransformUV(vertexIn.TexCoord , vertexIn.index);
	
	vertexOut.tangent = ReturnAnimatedNormal(vertexIn.tangent, vertexIn, local);
    vertexOut.tangent = normalize(mul(world_mat, vertexOut.tangent));
	
    vertexOut.normal = ReturnAnimatedNormal(vertexIn.normal, vertexIn , local);
    vertexOut.normal = normalize(mul(world_mat, vertexOut.normal));
	
    vertexOut.index = vertexIn.index;

	return vertexOut;
}
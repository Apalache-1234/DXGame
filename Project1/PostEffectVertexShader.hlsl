struct input
{
    float3 position : POSITION;

    float2 TexCoord : TEXCOORD;

    float3 normal : NORMAL;

    float4 color : COLOR;

    float3 tangent : TANGENT;

    float4 boneID : BONEID;

    float4 weight : WEIGHT;

    float parent : PARENT;
    
    float index : INDEX;
};

struct output
{
    float4 position : SV_POSITION;

    float2 TexCoord : TEXCOORD;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
    
    float index : INDEX;
};

#include <Header.hlsli> 

output main(input vertexIn)
{
    output vertexOut = (output) 0;

    vertexOut.color = color;

    vertexOut.position = float4(vertexIn.position, 0.5) * 2;

    vertexOut.TexCoord = vertexIn.TexCoord;

    vertexOut.tangent = vertexIn.tangent;

    vertexOut.normal = vertexIn.normal;
    
    vertexOut.index = vertexIn.index;

    return vertexOut;
}
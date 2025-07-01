struct output
{
    float4 position : SV_POSITION;

    float3 localPos : POSITION;
	
    float4x4 worldMat : WORLD_MATRIX;

    float lightIndex : LIGHT_INDEX0;

    float2 TexCoord : TEXCOORD;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
};

struct pixelOp
{
    float4 color : SV_TARGET0;
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;
    
    float4 dir = float4(1, 0, 0 , 0);
    
    float3 localPos = vertexOp.localPos;
    
    dir = normalize(mul(vertexOp.worldMat, dir));

    output_pixel.color = vertexOp.color;
    output_pixel.color.a = 0.4f;
    
    float cosAng = cos(SpotLights[vertexOp.lightIndex].angular);
    float dirDot = dot(SpotLights[vertexOp.lightIndex].direct, normalize(localPos));
    
    bool isInside = vertexOp.lightIndex > -0.01f && cosAng > dirDot;

    if (output_pixel.color.a == 0.0f ||  isInside)
        discard;

    return output_pixel;
}
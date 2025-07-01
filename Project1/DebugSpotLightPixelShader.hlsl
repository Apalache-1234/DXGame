struct output
{
    float4 position : SV_POSITION;
    
    float3 worldPos : POSITION;

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
    
    float3 localPos = mul(transpose(world_mat), float4(vertexOp.worldPos, 1));
    
    dir = normalize(mul(world_mat, dir));

    output_pixel.color = vertexOp.color * saturate(dot(dir, normalize(localPos)));

    if (output_pixel.color.a == 0.0f)
        discard;

    return output_pixel;
}
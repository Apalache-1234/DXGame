struct output
{
    float4 position : SV_POSITION;

    float4 worldPos : POSITION;

    float2 TexCoord : TEXCOORD;
    
    float4 lightPos : LIGHT_POS;

    float3 tangent : TANGENT;

    float3 normal : NORMAL;

    float4 color : COLOR0;
	
    float index : INDEX;
};

struct pixelOp
{
	float4 color: SV_TARGET0;
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

    float2 uv = vertexOp.TexCoord;

	output_pixel.color = tex.Sample(tex_sampler , uv) * vertexOp.color;

	vertexOp.normal = ApplyNormalMap(uv, vertexOp.normal, vertexOp.tangent);

	output_pixel.color = ApplyLightShading(output_pixel.color, vertexOp.worldPos ,vertexOp.normal , vertexOp.index);

	if (output_pixel.color.a == 0.0f) discard;

    float3 shadow_coord = vertexOp.lightPos.xyz / vertexOp.lightPos.w;
    
    shadow_coord.x = shadow_coord.x * 0.5 + 0.5;
    shadow_coord.y = -shadow_coord.y * 0.5 + 0.5;

    float shadowFactor = shadowMap.SampleCmpLevelZero
    (
    shadow_sampler,
    shadow_coord.xy,
    shadow_coord.z
    );
    
    output_pixel.color.rgb *= shadowFactor;
    
	return output_pixel;
}
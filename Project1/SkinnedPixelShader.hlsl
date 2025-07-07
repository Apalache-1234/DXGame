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
	float4 color: SV_TARGET0;
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

    float2 uv = vertexOp.TexCoord + float2(0 ,RealTime) * 0.01;

	output_pixel.color = tex.Sample(tex_sampler , uv) * vertexOp.color;

	vertexOp.normal = ApplyNormalMap(uv, vertexOp.normal, vertexOp.tangent);

	output_pixel.color = ApplyLightShading(output_pixel.color, vertexOp.worldPos ,vertexOp.normal);

	if (output_pixel.color.a == 0.0f) discard;


	return output_pixel;
}
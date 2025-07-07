struct output
{
    float4 position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
    float4 color : COLOR0;
};

struct pixelOp
{
	float4 color: SV_TARGET0;
};

TextureCube texC : register(t0);

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

	output_pixel.color = texC.Sample(tex_sampler, normalize(vertexOp.TexCoord)) * vertexOp.color;

	return output_pixel;
}
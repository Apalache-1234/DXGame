struct output
{
	float4 position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

struct pixelOp
{
	float4 color: SV_TARGET0;
};

Texture2D tex : register(t0);

SamplerState tex_sampler : register(s0);

pixelOp main(output vertexOp)
{
	pixelOp output_pixel = (pixelOp)0;

	output_pixel.color = tex.Sample(tex_sampler, normalize(vertexOp.TexCoord));

	return output_pixel;
}
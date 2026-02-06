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

struct pixelOp
{
    float4 color : SV_TARGET0;
};

cbuffer CardInfo : register(b34)
{
    float isHIt : packoffset(c0);
};

#include <Header.hlsli> 

pixelOp main(output vertexOp)
{
    pixelOp output_pixel = (pixelOp) 0;

    float2 uv = vertexOp.TexCoord;
    
    uv.y = 1 - uv.y;
    
    uv = frac(uv);

    output_pixel.color = tex.Sample(tex_sampler, uv) * vertexOp.color;

    if (output_pixel.color.a < 0.9)
        output_pixel.color.rgba = 1;
    
    vertexOp.normal = ApplyNormalMap(uv, vertexOp.normal, vertexOp.tangent);

    output_pixel.color = ApplyLightShading(output_pixel.color, vertexOp.worldPos, vertexOp.normal, vertexOp.index);
    
    float3 worldCenter = mul(world_mat, float4(0, 0, 0, 1)).xyz;

    float3 dirToCenter = normalize(cameraPos.xyz - worldCenter);

    float3 dirToPixel = normalize(cameraPos.xyz - vertexOp.worldPos);

    if (isHIt)
    {
        float d = saturate(dot(dirToPixel, dirToCenter));

        float hue = GameTime * 0.3 + d * 50;
        float3 effectColor = HsvToRgb(float3(hue % 1.0, 1, 1));

        float mask = pow(d, 20);
        output_pixel.color.xyz = lerp(effectColor, output_pixel.color.xyz, mask);
    }

    return output_pixel;
}
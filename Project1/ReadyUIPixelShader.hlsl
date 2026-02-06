struct output
{
    float4 position : SV_POSITION;

    float2 TexCoord : TEXCOORD;

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

    float2 uv = vertexOp.TexCoord;

    
    uv.y = 1.0 - uv.y;

    uv.y -= 0.5;

    
    float p = pow(uv.y, 3);
    
    if (uv.y > 0)
        p *= 1;
    else if (uv.y < 0)
        p *= -1;
    
    float val = sin((uv.x + GameTime) * 3.14f * 3) * p;
	
    uv.y += 0.5;
    
    uv.y += val * 1.5;
    
    uv *= 2;
    
    uv += 0.5f * (1.0f - 2);
    
    uv = saturate(uv);
    
    output_pixel.color = tex.Sample(tex_sampler, uv) * vertexOp.color;

    return output_pixel;
}
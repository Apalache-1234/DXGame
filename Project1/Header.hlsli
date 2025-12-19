cbuffer transform : register(b0)
{
	float4x4 world_mat : packoffset(c0);

	float4x4 view_mat : packoffset(c4);

	float4x4 proj_mat : packoffset(c8);

	float4 color : packoffset(c12);
}

cbuffer LightInfo : register(b1)
{
	float4 ambientLightCol : packoffset(c0);
	float4 directionalLightNum : packoffset(c1);
	float4 pointLightNum : packoffset(c2);
	float4 spotLightNum : packoffset(c3);
	float4 cameraPos : packoffset(c4);
}

cbuffer TimeInfo: register(b2)
{
    float GameTime : packoffset(c0);

    float GameDeltaTime : packoffset(c1);

    float RealTime : packoffset(c2);

    float RealDeltaTime : packoffset(c3);
};

float Hash(float2 p)
{
    p = frac(p * 0.3183099 + float2(0.71, 0.113));
    return frac(43758.5453 * dot(p, float2(1.0, 57.0)));
}

Texture2D tex : register(t0);
Texture2D normal : register(t1);

Texture2DArray<float> shadowMap : register(t100);

SamplerState tex_sampler : register(s0);
SamplerComparisonState shadow_sampler : register(s1);

const float bias = 0.001;
const float slopeBias = 0.004;

struct DirectionalLight
{
	float3 direct;

	float3 color;

	float power;
	
    float onShadow;
	
    float4x4 matVP;
	
    float shadowMapIndex;
};

struct PointLight
{
	float radius;

	float3 color;

	float3 pos;

	float power;
	
    float onShadow;
	
    float4x4 matVP;
	
    float shadowMapIndex;
};

struct SpotLight
{
	float angular;

	float3 direct;

	float3 color;

	float3 pos;

	float power;

	float length;
	
    float onShadow;
	
    float4x4 matVP;
	
    float shadowMapIndex;
};

struct Material
{
    float3 DIFFUSE;
    float3 SPECULAR;
    float3 AMBIENT;
    float3 REFLECTIVE;
    float3 TRANSPARENT;
    float3 EMISSIVE;
    float2 UVPOS;
    float2 UVSCALE;
    float4 TEXELSIZE;
};

StructuredBuffer<DirectionalLight> DirectionalLights : register(t10);

StructuredBuffer<PointLight> PointLights : register(t11);

StructuredBuffer<SpotLight> SpotLights : register(t12);

StructuredBuffer<Material> Materials : register(t30);

float4 normalizeColor(float4 col)
{
	float Max = max(col.x, max(col.y, col.z));

	if (Max > 1)
	{
		float sub = Max - 1;

		col.r -= sub * 0.1;
		col.g -= sub * 0.1;
		col.b -= sub * 0.1;
	}

	return col;
}

float2 TransformUV(float2 uv , int matIdx)
{
    return (uv + Materials[matIdx].UVPOS) * Materials[matIdx].UVSCALE;

}

float4 ReturnPhongLighting(float3 lightCol, float4 col, float4 pos, float3 lightDir, float3 normal, int matIdx)
{
	float3 outCol = lightCol + lightCol * col;

	lightCol = outCol * saturate(dot(lightDir, normal)) * Materials[matIdx].DIFFUSE;

	float3 toCamera = normalize(cameraPos.xyz - pos);

	float3 ref = -reflect(toCamera, normal);

    lightCol += outCol * pow(saturate(dot(ref, lightDir)), 100) * Materials[matIdx].SPECULAR;

	return float4(lightCol,1);
}

float4 DirectionalLightShading(float4 col, float4 pos, float3 normal, int matIdx)
{
    float4 sumCol = 0;

	for (int i = 0;i < directionalLightNum.x;i++)
	{
        float shadowFactor = 1;
		
        if (DirectionalLights[i].onShadow)
        {
            float4 lightPos = mul(DirectionalLights[i].matVP , pos);
			
            float3 shadow_coord = lightPos.xyz / lightPos.w;
    
            shadow_coord.x = shadow_coord.x * 0.5 + 0.5;
            shadow_coord.y = -shadow_coord.y * 0.5 + 0.5;

            float3 uvw = shadow_coord.xyz;
			
            uvw.z = DirectionalLights[i].shadowMapIndex;
			
            shadowFactor = shadowMap.SampleCmpLevelZero
            (
            shadow_sampler,
            uvw,
            shadow_coord.z
            );
			
            if (shadowFactor < 1)
                continue;

        }
		
        float3 lightDir = -normalize(DirectionalLights[i].direct);
		float pow = DirectionalLights[i].power;
		
        float4 light = ReturnPhongLighting(DirectionalLights[i].color , col , pos ,lightDir, normal , matIdx);

		sumCol += light * pow;

		sumCol = normalizeColor(sumCol);
    }

	return sumCol;
}

float4 PointLightShading(float4 col, float4 pos, float3 normal, int matIdx)
{
	float4 sumCol = 0;

	for (int i = 0;i < pointLightNum.x;i++)
	{
        if (PointLights[i].onShadow)
            continue;
		
		float3 lightDir = normalize(PointLights[i].pos - pos);
		float lenPow = pow(saturate((PointLights[i].radius - length(PointLights[i].pos - pos)) / PointLights[i].radius) , 3);
		float power = PointLights[i].power;

        float4 light = ReturnPhongLighting(PointLights[i].color, col, pos, lightDir, normal, matIdx) * lenPow;

		sumCol += light * power;

		sumCol = normalizeColor(sumCol);
	}

	return sumCol;
}

float4 SpotLightShading(float4 col, float4 pos, float3 normal, int matIdx)
{
	float4 sumCol = 0;

	for (int i = 0;i < spotLightNum.x;i++)
	{
        if (SpotLights[i].onShadow)
            continue;
		
		float3 lightDir = normalize(SpotLights[i].pos - pos);
		float lenPow = pow(saturate((SpotLights[i].length - length(SpotLights[i].pos - pos)) / SpotLights[i].length), 3);
		float power = SpotLights[i].power;

        float4 light = ReturnPhongLighting(SpotLights[i].color, col, pos, lightDir, normal, matIdx) * lenPow;

		float d = 1 - dot(SpotLights[i].direct, -lightDir) ;

		float di = 1 - cos(SpotLights[i].angular);

		light *= pow(1 - saturate(d / di), 3);

		sumCol += light * power;

		sumCol = normalizeColor(sumCol);
	}

	return sumCol;
}

float4 ApplyLightShading(float4 col, float4 pos, float3 normal,  int matIdx )
{
    float4 ambientCol = float4(Materials[matIdx].AMBIENT, 1) * col * float4(ambientLightCol.xyz, 1);

	float4 dirLightCol = DirectionalLightShading(col,pos, normal , matIdx);

    float4 pointLightCol = PointLightShading(col, pos, normal, matIdx);

    float4 spotLightCol = SpotLightShading(col, pos, normal, matIdx);

    float4 resultCol = dirLightCol + ambientCol + pointLightCol + spotLightCol;

	resultCol.a = 1;
	
	return resultCol;
}

float4 TransLocalToProj(float3 local)
{
	float4 localPos = float4(local , 1);

	float4 world = mul(world_mat, localPos);

	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	return proj;
}

float3 ApplyNormalMap(float2 uv , float3 nor, float3 tan)
{
	float3 bi = normalize(cross(nor , tan));

	float3x3 mat = float3x3(tan, bi, nor);

	mat = transpose(mat);

	float3 normalCol = (normal.Sample(tex_sampler, uv).xyz);

	normalCol.y = 1 - normalCol.y;

	normalCol = normalCol * 2 - 1;

	return normalize(mul(mat , normalCol));
}

float4 TransLocalToProj(float4 local)
{
	float4 world = mul(world_mat, local);

	float4 view = mul(view_mat, world);

	float4 proj = mul(proj_mat, view);

	return proj;
}


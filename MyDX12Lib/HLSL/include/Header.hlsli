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
	float cascadeShadowCount : packoffset(c5);
	float3 cascadeShadowDist : packoffset(c6);
	float3 fogColor : packoffset(c7);
	float fogRange : packoffset(c8);
	float fogFlag : packoffset(c9);
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
    float4x4 matVP[4];
	
	float3 direct;

	float3 color;

	float power;
	
    float onShadow;
	
    float shadowMapIndex;
	
    float3 padding;
};

struct PointLight
{
    float4x4 matVP;
	
	float radius;

	float3 color;

	float3 pos;

	float power;
	
    float onShadow;
	
    float shadowMapIndex;
	
    float2 padding;
};

struct SpotLight
{
    float4x4 matVP;
	
	float angular;

	float3 direct;

	float3 color;

	float3 pos;

	float power;

	float length;
	
    float onShadow;
	
    float shadowMapIndex;
	
    float2 padding;
};

struct Material
{
    float3 DIFFUSE;
    float3 SPECULAR;
    float3 AMBIENT;
    float3 REFLECTIVE;
    float REFLECTIVE_POW;
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

float3 HsvToRgb(float3 hsv)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
    return hsv.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), hsv.y);
}

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

    lightCol += outCol * pow(saturate(dot(ref, lightDir)), Materials[matIdx].REFLECTIVE_POW) * Materials[matIdx].SPECULAR;

	return float4(lightCol,1);
}

static const float2 poissonDisk[16] =
{
    float2(-0.94201624, -0.39906216),
    float2(0.94558609, -0.76890725),
    float2(-0.094184101, -0.92938870),
    float2(0.34495938, 0.29387760),
    float2(-0.91588581, 0.45771432),
    float2(-0.81544232, -0.87912464),
    float2(-0.38277543, 0.27676845),
    float2(0.97484398, 0.75648379),
    float2(0.44323325, -0.97511554),
    float2(0.53742981, -0.47373420),
    float2(-0.26496911, -0.41893023),
    float2(0.79197514, 0.19090188),
    float2(-0.24188840, 0.99706507),
    float2(-0.81409955, 0.91437590),
    float2(0.19984126, 0.78641367),
    float2(0.14383161, -0.14100790)
};

float CalcShadowPoisson(float3 shadowCoord,float depth, float bias)
{
    float shadow = 0.0f;
    float mapSize = 2048.0f;
    float texelSize = 1.0f / mapSize;

    float filterRadius = 2.0f;

    for (int i = 0; i < 16; ++i)
    {
        float2 offset = poissonDisk[i] * texelSize * filterRadius;
		
        float3 coord = shadowCoord;
		
        coord.xy += offset;
		
        shadow += shadowMap.SampleCmpLevelZero(
            shadow_sampler,
            coord,
            depth - bias
        ).r;
    }

    return shadow / 16.0f;
}

float4 DirectionalLightShading(float4 col, float4 pos, float3 normal, int matIdx)
{
    float4 sumCol = 0;

	for (int i = 0;i < directionalLightNum.x;i++)
	{
        float shadowFactor = 1;
		
        if (DirectionalLights[i].onShadow)
        {
            float viewDepth = mul(view_mat, pos).z;
			
            float shadowMapIndex = DirectionalLights[i].shadowMapIndex;
            float offset = 0;
			
            if (cascadeShadowCount >= 4 && viewDepth >= cascadeShadowDist[2])
                offset = 3;
            else if (cascadeShadowCount >= 3 && viewDepth >= cascadeShadowDist[1])
                offset = 2;
            else if (cascadeShadowCount >= 2 && viewDepth >= cascadeShadowDist[0])
                offset = 1;
			
			float4 lightPos = mul(DirectionalLights[i].matVP[offset], pos);
			
            float3 shadow_coord = lightPos.xyz / lightPos.w;
    
            shadow_coord.x = shadow_coord.x * 0.5 + 0.5;
            shadow_coord.y = -shadow_coord.y * 0.5 + 0.5;

            float3 uvw = shadow_coord.xyz;
			
            uvw.z = DirectionalLights[i].shadowMapIndex + offset;
			
            shadowFactor = CalcShadowPoisson(uvw, shadow_coord.z, 0);
        }
		
        float3 lightDir = -normalize(DirectionalLights[i].direct);
		float pow = DirectionalLights[i].power;
		
        float4 light = ReturnPhongLighting(DirectionalLights[i].color , col , pos ,lightDir, normal , matIdx);

		sumCol += light * pow * shadowFactor;

		sumCol = normalizeColor(sumCol);
    }

	return sumCol;
}

float4 PointLightShading(float4 col, float4 pos, float3 normal, int matIdx)
{
	float4 sumCol = 0;

	for (int i = 0;i < pointLightNum.x;i++)
	{
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
        float shadowFactor = 1;
		
        if (SpotLights[i].onShadow)
        {
            float viewDepth = mul(view_mat, pos).z;
			
            float shadowMapIndex = SpotLights[i].shadowMapIndex;
     
            float4 lightPos = mul(SpotLights[i].matVP, pos);
			
            float3 shadow_coord = lightPos.xyz / lightPos.w;
    
            shadow_coord.x = shadow_coord.x * 0.5 + 0.5;
            shadow_coord.y = -shadow_coord.y * 0.5 + 0.5;

            float3 uvw = shadow_coord.xyz;
			
            uvw.z = SpotLights[i].shadowMapIndex;
			
            shadowFactor = CalcShadowPoisson(uvw, shadow_coord.z, 0);
        }
		
		float3 lightDir = normalize(SpotLights[i].pos - pos);
		float lenPow = pow(saturate((SpotLights[i].length - length(SpotLights[i].pos - pos)) / SpotLights[i].length), 3);
		float power = SpotLights[i].power;

        float4 light = ReturnPhongLighting(SpotLights[i].color, col, pos, lightDir, normal, matIdx) * lenPow;

		float d = 1 - dot(SpotLights[i].direct, -lightDir) ;

		float di = 1 - cos(SpotLights[i].angular);

		light *= pow(1 - saturate(d / di), 3);

        sumCol += light * power * shadowFactor;

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
	
    float3 len = length(pos - cameraPos);
	
	if(fogFlag)
        resultCol.xyz = lerp(resultCol, fogColor, saturate(len / fogRange));
	
    resultCol.a = col.a;
	
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

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}


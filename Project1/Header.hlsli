#pragma once

cbuffer transform : register(b0)
{
	float4x4 world_mat : packoffset(c0);

	float4x4 view_mat : packoffset(c4);

	float4x4 proj_mat : packoffset(c8);

	float4 color : packoffset(c12);
}

cbuffer LightInfo : register(b1)
{
	float4 ambientLight : packoffset(c0);
	float4 directionalLightNum : packoffset(c1);
}

struct DirectionalLight
{
	float3 direct;

	float3 color;

	float reflection;

	float num;
};

StructuredBuffer<DirectionalLight> DirectionalLights : register(t10);

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

float4 DirectionalLightShading(float4 col, float3 normal)
{
    float4 sumCol = 0;
	
	for (int i = 0;i < directionalLightNum.x;i++)
	{
		float3 lightDir = normalize(DirectionalLights[i].direct);
		float ref = DirectionalLights[i].reflection;
		
        float4 light = float4((DirectionalLights[i].color + col) * max(dot(-lightDir, normal), 0), 1);

		sumCol += light * ref;

		sumCol = normalizeColor(sumCol);
    }

	return sumCol;
}

float4 ApplyLightShading(float4 col , float3 normal)
{
	float4 ambientCol = col * float4(ambientLight.xyz , 1);

	float4 lightCol = DirectionalLightShading(col, normal);

	return lightCol + ambientCol;
}


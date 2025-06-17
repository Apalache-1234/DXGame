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

float4 DirectionalLightShading(float4 col, float3 normal)
{
	for (int i = 0;i < directionalLightNum.x;i++)
	{
		float3 lightDir = normalize(DirectionalLights[i].direct);
		float3 ref = normalize(DirectionalLights[i].reflection);

		col = col + float4(ref * DirectionalLights[i].color * clamp(dot(-lightDir, normal), 0, 1), 1);
	}

	return col;
}

float4 ApplyLightShading(float4 col , float3 normal)
{
	col = col * float4(ambientLight.xyz , 1);

	col = DirectionalLightShading(col, normal);

	return col;
}
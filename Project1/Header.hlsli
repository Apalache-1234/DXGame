#pragma once

cbuffer transform : register(b0)
{
	float4x4 world_mat : packoffset(c0);

	float4x4 view_mat : packoffset(c4);

	float4x4 proj_mat : packoffset(c8);

	float4 color : packoffset(c12);
}
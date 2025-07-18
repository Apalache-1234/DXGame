#include "PERadicalBlur.h"

PERadicalBlur::PERadicalBlur(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PERadicalBlur::~PERadicalBlur()
{
}

std::type_index PERadicalBlur::GetBaseType()
{
	return typeid(PostEffect);
}

void PERadicalBlur::InitPostEffect()
{
	LoadPixelShader(L"RadicalBlurPixelShader.cso");

	CreateConstantBuffer<RadicalBlurInfo>();
}

bool PERadicalBlur::MapPECB()
{
	void* ptr = nullptr;

	hr = postEffectCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポストエフェクトの情報が収納されたMBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &info, sizeof(RadicalBlurInfo));

	postEffectCB->Unmap(0, nullptr);

	return true;
}

int PERadicalBlur::GetSample()
{
	return info.sampleCount;
}

void PERadicalBlur::SetSample(int count)
{
	count = std::max(count,1);

	info.sampleCount = count;
}

float PERadicalBlur::GetBlurStrength()
{
	return info.blurStrength;
}

void PERadicalBlur::SetBlurStrength(float str)
{
	str = std::max(str, 0.0f);

	info.blurStrength = str;
}

XMFLOAT2 PERadicalBlur::GetScreenCenter()
{
	return info.screenCenter;
}

void PERadicalBlur::SetScreenCenter(XMFLOAT2 pos)
{
	info.screenCenter = pos;
}

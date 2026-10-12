#include "PCH/pch.h"
#include "PENoiseFilter.h"

PENoiseFilter::PENoiseFilter(shared_ptr<Entity> ent) : PostEffect(ent)
{
}

PENoiseFilter::~PENoiseFilter()
{
}

void PENoiseFilter::SetIntensify(float in)
{
	info.intensify = std::clamp(in, 0.0f, 1.0f);
}

float PENoiseFilter::GetIntensify()
{
	return info.intensify;
}

void PENoiseFilter::SetBlockSize(float size)
{
	info.blockSize = std::clamp(size , 0.0f,1.0f);
}

float PENoiseFilter::GetBlockSize()
{
	return info.blockSize;
}

void PENoiseFilter::InitPostEffect()
{
	LoadPixelShader(L"NoiseFilterPixelShader.cso");

	CreateConstantBuffer<NoiseFilterInfo>();
}

bool PENoiseFilter::MapPECB()
{
	void* ptr = nullptr;

	hr = postEffectCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポストエフェクトの情報が収納されたMBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &info, sizeof(NoiseFilterInfo));

	postEffectCB->Unmap(0, nullptr);

	return true;
}

#include "PEMosaicFilter.h"

PEMosaicFilter::PEMosaicFilter(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PEMosaicFilter::~PEMosaicFilter()
{
}

std::type_index PEMosaicFilter::GetBaseType()
{
	return typeid(PostEffect);
}

void PEMosaicFilter::InitPostEffect()
{
	LoadPixelShader(L"MosaicFilterPixelShader.cso");

	CreateConstantBuffer<MosaicFilterInfo>();
}

bool PEMosaicFilter::MapPECB()
{
	void* ptr = nullptr;

	hr = postEffectCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポストエフェクトの情報が収納されたMBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &info, sizeof(MosaicFilterInfo));

	postEffectCB->Unmap(0, nullptr);

	return true;
}

float PEMosaicFilter::GetIntensify()
{
	return info.intensify;
}

void PEMosaicFilter::SetIntensify(float val)
{
	val = std::max(val , 0.0001f);

	info.intensify = val;
}

#include "PEEdgeFilter.h"

PEEdgeFilter::PEEdgeFilter(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PEEdgeFilter::~PEEdgeFilter()
{
}

std::type_index PEEdgeFilter::GetBaseType()
{
	return typeid(PostEffect);
}

void PEEdgeFilter::InitPostEffect()
{
	LoadPixelShader(L"EdgeFilterPixelShader.cso");

	CreateConstantBuffer<EdgeFilterInfo>();
}

bool PEEdgeFilter::MapPECB()
{
	void* ptr = nullptr;

	hr = postEffectCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポストエフェクトの情報が収納されたMBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &info, sizeof(EdgeFilterInfo));

	postEffectCB->Unmap(0, nullptr);

	return true;
}

float PEEdgeFilter::GetTransition()
{
	return info.transition;
}

void PEEdgeFilter::SetTransition(float val)
{
	val = std::clamp(val , 0.0f ,1.0f);

	info.transition = val;
}

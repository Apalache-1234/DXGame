#include "PEVerticalGaussianBlur.h"

PEVerticalGaussianBlur::PEVerticalGaussianBlur(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PEVerticalGaussianBlur::~PEVerticalGaussianBlur()
{
}

std::type_index PEVerticalGaussianBlur::GetBaseType()
{
	return typeid(PostEffect);
}

void PEVerticalGaussianBlur::InitPostEffect()
{
	LoadPixelShader(L"VerticalGaussianBlurPixelShader.cso");

	CreateConstantBuffer<GaussianBlurInfo>();
}

bool PEVerticalGaussianBlur::MapPECB()
{
	void* ptr = nullptr;

	hr = postEffectCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ガウシアンブラーの情報が収納されたGBIのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &info, sizeof(GaussianBlurInfo));

	postEffectCB->Unmap(0, nullptr);

	return true;
}

int PEVerticalGaussianBlur::GetKernelRadius()
{
	return info.kernelRadius;
}

void PEVerticalGaussianBlur::SetKernelRadius(int count)
{
	count = std::max(count, 1);

	info.kernelRadius = count;
}

float PEVerticalGaussianBlur::GetSigma()
{
	return info.sigma;
}

void PEVerticalGaussianBlur::SetSigma(float sig)
{
	sig = std::max(sig, 0.001f);

	info.sigma = sig;
}

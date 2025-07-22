#include "PEHorizonGaussianBlur.h"

PEHorizonGaussianBlur::PEHorizonGaussianBlur(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PEHorizonGaussianBlur::~PEHorizonGaussianBlur()
{
}

std::type_index PEHorizonGaussianBlur::GetBaseType()
{
	return typeid(PostEffect);
}

void PEHorizonGaussianBlur::InitPostEffect()
{
	LoadPixelShader(L"HorizonGaussianBlurPixelShader.cso");

	CreateConstantBuffer<GaussianBlurInfo>();
}

bool PEHorizonGaussianBlur::MapPECB()
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

int PEHorizonGaussianBlur::GetKernelRadius()
{
	return info.kernelRadius;
}

void PEHorizonGaussianBlur::SetKernelRadius(int count)
{
	count = std::max(count , 1);

	info.kernelRadius = count;
}

float PEHorizonGaussianBlur::GetSigma()
{
	return info.sigma;
}

void PEHorizonGaussianBlur::SetSigma(float sig)
{
	sig = std::max(sig, 0.001f);

	info.sigma = sig;
}

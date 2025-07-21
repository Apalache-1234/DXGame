#include "PEGaussianBlur.h"

PEGaussianBlur::PEGaussianBlur(shared_ptr<Entity> ent) : PostEffect(ent)
{
}


PEGaussianBlur::~PEGaussianBlur()
{
}

std::type_index PEGaussianBlur::GetBaseType()
{
	return typeid(PostEffect);
}

void PEGaussianBlur::InitPostEffect()
{
	LoadPixelShader(L"GaussianBlurPixelShader.cso");

	CreateConstantBuffer<GaussianBlurInfo>();

	passNum = 2;
}

bool PEGaussianBlur::MapPECB()
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

	info.direction = std::fmod(info.direction + 1 , 2);

	return true;
}

int PEGaussianBlur::GetKernelRadius()
{
	return info.kernelRadius;
}

void PEGaussianBlur::SetKernelRadius(int count)
{
	count = std::max(count , 1);

	info.kernelRadius = count;
}

float PEGaussianBlur::GetSigma()
{
	return info.sigma;
}

void PEGaussianBlur::SetSigma(float sig)
{
	sig = std::max(sig, 0.001f);

	info.sigma = sig;
}

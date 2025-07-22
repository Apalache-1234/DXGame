#ifndef PS_HORIZON_GAUSSIAN_BLUR
#define PS_HORIZON_GAUSSIAN_BLUR

#include "PostEffect.h"

class PEHorizonGaussianBlur : public PostEffect
{
public:
    PEHorizonGaussianBlur(shared_ptr<Entity> ent);

    ~PEHorizonGaussianBlur();

    std::type_index GetBaseType() override;

    int GetKernelRadius();

    void SetKernelRadius(int count);

    float GetSigma();

    void SetSigma(float sig);
private:
    void InitPostEffect() override;

    bool MapPECB() override;

    struct alignas(256) GaussianBlurInfo
    {
        float sigma = 1;
        XMFLOAT3 padding1;
        float kernelRadius = 4;
        XMFLOAT3 padding2;
    };

    int currentPass = 0;

    GaussianBlurInfo info;

    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> vsBlob;
};
#endif

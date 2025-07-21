#ifndef PS_GAUSSIAN_BLUR
#define PS_GAUSSIAN_BLUR

#include "PostEffect.h"

class PEGaussianBlur : public PostEffect
{
public:
    PEGaussianBlur(shared_ptr<Entity> ent);

    ~PEGaussianBlur();

    std::type_index GetBaseType() override;

    int GetKernelRadius();

    void SetKernelRadius(int count);

    float GetSigma();

    void SetSigma(float sig);
private:
    void InitPostEffect() override;

    bool MapPECB() override;

    ComPtr<ID3DBlob> psVerticaBlob;

    struct alignas(256) GaussianBlurInfo
    {
        float sigma = 1;
        XMFLOAT3 padding1;
        float kernelRadius = 4;
        XMFLOAT3 padding2;
        float direction = 0;
        XMFLOAT3 padding3;
    };

    GaussianBlurInfo info;
};
#endif

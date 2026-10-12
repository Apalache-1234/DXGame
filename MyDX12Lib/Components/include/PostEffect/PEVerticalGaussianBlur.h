#ifndef PS_VERTICAL_GAUSSIAN_BLUR
#define PS_VERTICAL_GAUSSIAN_BLUR

#include "PostEffect.h"

class PEVerticalGaussianBlur : public PostEffect
{
public:
    PEVerticalGaussianBlur(shared_ptr<Entity> ent);

    ~PEVerticalGaussianBlur();

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

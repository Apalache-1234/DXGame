#ifndef PS_NOISE_BLUR
#define PS_NOISE_BLUR

#include "PostEffect.h"

class PENoiseFilter : public PostEffect
{
public:
    PENoiseFilter(shared_ptr<Entity> ent);

    ~PENoiseFilter();

    void SetIntensify(float in);

    float GetIntensify();

    void SetBlockSize(float size);

    float GetBlockSize();
private:
    void InitPostEffect() override;

    bool MapPECB() override;

    struct alignas(256) NoiseFilterInfo
    {
        float intensify = 0.0f;
        DirectX::XMFLOAT3 padding1;
        float blockSize = 0.001f;
        DirectX::XMFLOAT3 padding2;
    };

    NoiseFilterInfo info;
};
#endif

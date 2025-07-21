#ifndef PS_RADICAL_BLUR
#define PS_RADICAL_BLUR

#include "PostEffect.h"

class PERadicalBlur : public PostEffect
{
public:
    PERadicalBlur(shared_ptr<Entity> ent);

    ~PERadicalBlur();

    std::type_index GetBaseType() override;

    int GetSample();

    void SetSample(int count);

    float GetBlurStrength();

    void SetBlurStrength(float str);

    XMFLOAT2 GetScreenCenter();

    void SetScreenCenter(XMFLOAT2 pos);
private:
    void InitPostEffect() override;

    bool MapPECB() override;

    struct alignas(256) RadicalBlurInfo
    {
        DirectX::XMFLOAT2 screenCenter = { 0.5f, 0.5f }; 
        DirectX::XMFLOAT2 padding1;
        float blurStrength = 0.5f; 
        DirectX::XMFLOAT3 padding2;
        int sampleCount = 16; 
        DirectX::XMFLOAT3 padding3;
    };

    RadicalBlurInfo info;
};
#endif

#ifndef PS_MOSAIC_FILTER
#define PS_MOSAIC_FILTER

#include "PostEffect.h"

class PEMosaicFilter : public PostEffect
{
public:
    PEMosaicFilter(shared_ptr<Entity> ent);

    ~PEMosaicFilter();

    float GetIntensify();

    void SetIntensify(float val);
private:
    void InitPostEffect() override;

    bool MapPECB() override;

    struct alignas(256) MosaicFilterInfo
    {
        float intensify = 1;

        DirectX::XMFLOAT3 padding1;
    };

    MosaicFilterInfo info;
};
#endif

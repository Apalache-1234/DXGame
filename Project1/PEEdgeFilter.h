#ifndef PS_EDGE_FILTER
#define PS_EDGE_FILTER

#include "PostEffect.h"
class PEEdgeFilter : public PostEffect
{
public:
	PEEdgeFilter(shared_ptr<Entity> ent);

	~PEEdgeFilter();

	std::type_index GetBaseType() override;

    float GetTransition();

    void SetTransition(float val);

private:
    void InitPostEffect() override;

    bool MapPECB() override;

    struct alignas(256) EdgeFilterInfo
    {
        float transition = 1;

        DirectX::XMFLOAT3 padding1;
    };

    EdgeFilterInfo info;
};
#endif

#ifndef DEBUGRENDERER_H
#define DEBUGRENDERER_H

#include "Renderer.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class DebugRenderer : public Renderer
{
public:
	virtual ~DebugRenderer();
protected:
	DebugRenderer(std::shared_ptr<Entity> ent);

	bool InitIVB();

	void virtual InitDebugRenderer();

	struct Instance
	{
		XMMATRIX world;
		XMFLOAT4 color;
	};

	std::vector<Instance> instances;

	bool MapDVB();

	D3D12_VERTEX_BUFFER_VIEW IVBV;
private:
	void AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList) override;

	void InitRenderer() override;

	bool InitDebugVertexBuffer();

	bool InitDBV();

	ComPtr<ID3D12Resource> IVB;
};

#endif



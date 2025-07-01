#ifndef DEBUGRENDERER_H
#define DEBUGRENDERER_H

#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#include "windows.h"
#include <sstream>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "Window.h"
#include "DXRenderSystem.h"
#include "Utility.h"
#include "Renderer.h"
#include "Collider.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class DebugRenderer : public Renderer
{
public:
	DebugRenderer(std::shared_ptr<Entity> ent);

	virtual ~DebugRenderer();
protected:
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



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
#include <d3dcompiler.h>
#include "Window.h"
#include "DXRenderSystem.h"
#include "Utility.h"
#include "Renderer.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class DebugRenderer : public Renderer
{
public:
	DebugRenderer(std::shared_ptr<Entity> ent);

	~DebugRenderer();

	bool InitDebugVertexBuffer();

	bool MapDVB();

	bool InitDBV();
protected:
	void AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList) override;

	void Update() override;

	void InitRenderer() override;
private:
	void RenderDebugCollider();

	HRESULT hr;

	struct DebugColMat
	{
		XMMATRIX world;
		XMFLOAT4 color;
	};

	ComPtr<ID3D12Resource> colliderDVB;

	D3D12_VERTEX_BUFFER_VIEW colliderDVBV;

	std::vector<DebugColMat> debugColMat;
};

#endif



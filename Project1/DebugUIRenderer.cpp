#include "DebugUIRenderer.h"
#include "UIRenderer.h"

DebugUIRenderer::DebugUIRenderer(std::shared_ptr<Entity> ent) : DebugRenderer::DebugRenderer(ent)
{
}

DebugUIRenderer::~DebugUIRenderer()
{
}

void DebugUIRenderer::Update()
{
}

void DebugUIRenderer::InitDebugRenderer()
{
	CreateMesh(MeshLoader::PLANE);

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = {};
	dsDesc.BackFace = {};

	SetDepthStencilDesc(dsDesc);

	auto comp = GameManager::GetInstance().GetComponentList<UIRenderer>();

	for (int i = 0; i < comp.size(); i++)
	{
		Instance inst;

		inst.world = comp[i]->GetWorldMat();
		inst.color = DirectX::XMFLOAT4(0, 1, 0, 1);

		instances.emplace_back(inst);
	}

}

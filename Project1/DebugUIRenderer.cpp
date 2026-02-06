#include "DebugUIRenderer.h"
#include "UIRenderer.h"
#include "Camera.h"

DebugUIRenderer::DebugUIRenderer(std::shared_ptr<Entity> ent) : DebugRenderer::DebugRenderer(ent)
{
}

DebugUIRenderer::~DebugUIRenderer()
{
}

std::type_index DebugUIRenderer::GetBaseType()
{
	return typeid(DebugRenderer);
}

void DebugUIRenderer::Update()
{
	InitDebugRenderer();

	if (!InitIVB())
		std::cerr << "DebugUIRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;

	MapDVB();
}

void DebugUIRenderer::InitDebugRenderer()
{
	SetPrimitiveType(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	instances.clear();

	CreateMesh(MeshLoader::BOX);

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

	auto comp = SystemManager::GetInstance().GetComponentList<UIRenderer>();

	for (int i = 0; i < comp.size(); i++)
	{
		Instance inst;

		DirectX::XMFLOAT2 vec = comp[i]->GetResultScale();

		auto t = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorAdd(comp[i]->attachFrom.lock()->GetWorldPosition() , comp[i]->colPosition));
		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorMultiply(DirectX::XMLoadFloat2(&vec), comp[i]->colScale));

		inst.world = s * t;
		inst.color = DirectX::XMFLOAT4(0, 1, 0, 1);

		instances.emplace_back(inst);
	}

	instanceCount = instances.size();

	trans.view = XMMatrixLookAtLH(Utility::zero3D, Utility::forward, DirectX::XMVectorSet(0, 1, 0, 0));

	trans.projection = DirectX::XMMatrixOrthographicLH(Window::width, Window::height, Camera::GetInstance().GetNearZ(), Camera::GetInstance().GetFarZ());
}

#include "SkyBox.h"
#include "Utility.h"
#include "Camera.h"

SkyBox::SkyBox(std::shared_ptr<Entity> ent) : Renderer::Renderer(ent)
{
}

SkyBox::~SkyBox()
{
}

std::type_index SkyBox::GetBaseType()
{
	return typeid(Renderer);
}

void SkyBox::Update()
{
	auto position_mat = XMMatrixTranslationFromVector(DirectX::XMVectorZero());

	auto rotationMat = XMMatrixRotationY(0);

	auto scale_mat = XMMatrixScalingFromVector(attachFrom.lock()->GetWorldScale());

	trans.world = scale_mat * rotationMat * position_mat;

	auto camPos = Camera::GetInstance().GetPosition();

	trans.view = XMMatrixLookAtLH(camPos, Camera::GetInstance().GetTargetPos(), DirectX::XMVectorSet(0, 1, 0, 0));

	trans.view.r[3] = DirectX::XMVectorSet(0, 0, 0, 0);

	trans.projection = XMMatrixPerspectiveFovLH(Camera::GetInstance().GetFov(), Camera::GetInstance().GetAspect(), Camera::GetInstance().GetNearZ(), Camera::GetInstance().GetFarZ());

	DirectX::XMStoreFloat4(&trans.color , color) ;
}

void SkyBox::InitRenderer()
{
	CreateTexture();

	attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(90,90,90,0));

	LoadVertexShader(L"SkyBoxVertexShader.cso");
	LoadPixelShader(L"SkyBoxPixelShader.cso");

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = {};
	dsDesc.BackFace = {};

	SetDepthStencilDesc(dsDesc);
}

bool SkyBox::CreateTexture()
{
	auto resource = TextureLoader::GetInstance().LoadTexture("Textures/SkyBox.dds");

	auto textureDesc = resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	desc.Format = textureDesc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.TextureCube.MipLevels = textureDesc.MipLevels;
	desc.TextureCube.MostDetailedMip = 0;
	desc.TextureCube.ResourceMinLODClamp = 0;

	ApplyTextureOnMesh(0, Renderer::DIFFUSE, resource , desc);

	return true;
}

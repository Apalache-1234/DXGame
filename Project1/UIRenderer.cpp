#include "UIRenderer.h"
#include "Utility.h"
#include "Camera.h"
#include "KeyInput.h"
UINT UIRenderer::GetImageWidth()
{
	return imageWidth;
}

UINT UIRenderer::GetImageHeight()
{
	return imageHeight;
}

void UIRenderer::InitRenderer()
{
	CreateTexture("Textures/error.dds");

	CreateMesh(MeshLoader::PLANE);

	LoadVertexShader(L"UIVertexShader.cso");
	LoadPixelShader(L"UIPixelShader.cso");

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
}

void UIRenderer::Update()
{
	MakeWorldMat();

	CheckPointerInbound();
}

DirectX::XMMATRIX UIRenderer::GetWorldMat()
{
	auto worldPos = attachFrom->GetWorldPosition();

	float posX = DirectX::XMVectorGetX(worldPos);
	float posY = DirectX::XMVectorGetY(worldPos);

	auto position_mat = XMMatrixTranslation(posX, posY, 0);

	auto worldZ = DirectX::XMVectorGetZ(attachFrom->GetWorldRotation());
	auto localZ = DirectX::XMVectorGetZ(attachFrom->GetLocalRotation());

	worldZ = DirectX::XMConvertToRadians(worldZ);
	localZ = DirectX::XMConvertToRadians(localZ);

	auto rotation_mat = DirectX::XMMatrixRotationZ(worldZ + localZ);

	auto scale = attachFrom->GetScale();

	float scaleX = DirectX::XMVectorGetX(scale) * imageWidth;
	float scaleY = DirectX::XMVectorGetY(scale) * imageHeight;

	auto scale_mat = XMMatrixScaling(scaleX, scaleY, 1);

	return scale_mat * rotation_mat * position_mat;
}

void UIRenderer::MakeWorldMat()
{
	auto worldPos = attachFrom->GetWorldPosition();

	float posX = DirectX::XMVectorGetX(worldPos);
	float posY = DirectX::XMVectorGetY(worldPos);

	auto position_mat = XMMatrixTranslation(posX, posY, 0);

	auto worldZ = DirectX::XMVectorGetZ(attachFrom->GetWorldRotation());
	auto localZ = DirectX::XMVectorGetZ(attachFrom->GetLocalRotation());

	worldZ = DirectX::XMConvertToRadians(worldZ);
	localZ = DirectX::XMConvertToRadians(localZ);

	auto rotation_mat = DirectX::XMMatrixRotationZ(worldZ + localZ);

	auto scale = attachFrom->GetScale();

	float scaleX = DirectX::XMVectorGetX(scale) * imageWidth;
	float scaleY = DirectX::XMVectorGetY(scale) * imageHeight;

	auto scale_mat = XMMatrixScaling(scaleX, scaleY, 1);

	trans.world = scale_mat * rotation_mat * position_mat;

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	auto color_normalized = XMFLOAT4(colR, colG, colB, colA);

	trans.view = XMMatrixLookAtLH(Utility::zero3D, Utility::forward, DirectX::XMVectorSet(0, 1, 0, 0));

	trans.projection = DirectX::XMMatrixOrthographicLH(Window::width, Window::height, Camera::nearZ, Camera::farZ);

	trans.color = color_normalized;
}

XMFLOAT2 UIRenderer::GetResultScale()
{
	auto scale = attachFrom->GetWorldScale();

	return XMFLOAT2(imageWidth * DirectX::XMVectorGetX(scale) , imageHeight * DirectX::XMVectorGetY(scale));
}

void UIRenderer::CheckPointerInbound()
{
	auto input = KeyInput::GetInstance();

	auto pointer = input.GetMousePos();

	auto pos = DirectX::XMVectorAdd(attachFrom->GetWorldPosition() , colPosition);

	auto scale = GetResultScale();

	scale.x *= DirectX::XMVectorGetX(colScale);
	scale.y *= DirectX::XMVectorGetY(colScale);

	auto x = scale.x / 2 + Window::width / 2 + DirectX::XMVectorGetX(pos);
	auto xM = -scale.x / 2 + Window::width / 2 + DirectX::XMVectorGetX(pos);
	auto y = scale.y / 2 + Window::height / 2 + DirectX::XMVectorGetY(pos);
	auto yM = -scale.y / 2 + Window::height / 2 + DirectX::XMVectorGetY(pos);

	auto pointX = pointer.x;
	auto pointY = pointer.y;

	if (x >= pointX && xM <= pointX && y >= pointY && yM <= pointY)
	{
		isOnMousePointer = true;

		if (input.GetKey(Key::MOUSE_LEFT))
		{
			attachFrom->OnMouseClick();
		}
	}
	else
	{
		if (isOnMousePointer == true)
		{
			attachFrom->OnMousePointerOut();
		}

		isOnMousePointer = false;
	}
}

UIRenderer::UIRenderer(std::shared_ptr<Entity> ent) : Renderer::Renderer(ent)
{
}

UIRenderer::~UIRenderer()
{
}

bool UIRenderer::CreateTexture(const std::string fileName)
{
	texHandle = heapManager.GetHandle();

	auto resource = TextureLoader::GetInstance().LoadTexture(fileName);

	auto textureDesc = resource->GetDesc(); 

	imageWidth = textureDesc.Width;
	imageHeight = textureDesc.Height;

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = textureDesc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.Texture2D.MipLevels = textureDesc.MipLevels;
	desc.Texture2D.PlaneSlice = 0;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(resource.Get(), &desc, texHandle->cpuHandle);

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(resource.Get(), &desc, texHandle->cpuHandle);

	texHandles.clear();

	texHandles.emplace_back(std::move(texHandle));

	return true;
}

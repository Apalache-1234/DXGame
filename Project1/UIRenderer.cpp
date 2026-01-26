#include "UIRenderer.h"
#include "Utility.h"
#include "Camera.h"
#include "KeyInput.h"
#include "ScriptComponent.h"

UINT UIRenderer::GetImageWidth()
{
	return imageWidth;
}

UINT UIRenderer::GetImageHeight()
{
	return imageHeight;
}

void UIRenderer::SetImageWidth(UINT size)
{
	imageWidth = size;
}

void UIRenderer::SetImageHeight(UINT size)
{
	imageHeight = size;
}

std::type_index UIRenderer::GetBaseType()
{
	return typeid(Renderer);
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
	trans = GetTransform();

	CheckPointerInbound();
}

Transform UIRenderer::GetTransform()
{
	Transform transform;

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

	transform.world = scale_mat * rotation_mat * position_mat;

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	auto color_normalized = XMFLOAT4(colR, colG, colB, colA);

	transform.view = XMMatrixLookAtLH(Utility::zero3D, Utility::forward, DirectX::XMVectorSet(0, 1, 0, 0));

	transform.projection = DirectX::XMMatrixOrthographicLH(Window::width, Window::height, Camera::GetInstance().GetNearZ(), Camera::GetInstance().GetFarZ());

	transform.color = color_normalized;

	return transform;
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
	auto xN = -scale.x / 2 + Window::width / 2 + DirectX::XMVectorGetX(pos);
	auto y = scale.y / 2 + Window::height / 2 + DirectX::XMVectorGetY(pos);
	auto yN = -scale.y / 2 + Window::height / 2 + DirectX::XMVectorGetY(pos);

	auto pointX = pointer.x;
	auto pointY = pointer.y;

	auto scripts = attachFrom->GetComponents<ScriptComponent>();

	if (x >= pointX && xN <= pointX && y >= pointY && yN <= pointY)
	{

		if (isOnMousePointer == false)
		{
			for (auto script : scripts)
				script->OnMousePointerIn();
		}

		isOnMousePointer = true;

		for (auto script : scripts)
			script->OnMousePointerOn();

		if (input.GetKey(Key::MOUSE_LEFT))
		{
			for (auto script : scripts)
				script->OnMousePointerClick();
		}
	}
	else
	{
		if (isOnMousePointer == true)
		{
			for (auto script : scripts)
				script->OnMousePointerOut();
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

D3D12_CPU_DESCRIPTOR_HANDLE UIRenderer::GetTextureCPUHandle()
{
	return texHandles[0]->cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE UIRenderer::GetTextureGPUHandle()
{
	return texHandles[0]->gpuHandle;
}

bool UIRenderer::CreateTexture(const std::string fileName)
{
	auto resource = TextureLoader::GetInstance().LoadTexture(fileName);

	imageWidth = resource->GetDesc().Width;
	imageHeight = resource->GetDesc().Height;

	ApplyTextureOnMesh(0, Renderer::DIFFUSE, resource);

	return true;
}

#include "DebugRenderer.h"
#include "Collider.h"
#include "Camera.h"

DebugRenderer::DebugRenderer(std::shared_ptr<Entity> ent) : Renderer::Renderer(ent)
{
}

DebugRenderer::~DebugRenderer()
{
}

bool DebugRenderer::InitDebugVertexBuffer()
{
	UINT bufferSize = sizeof(DebugColMat) * debugColMat.size();

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = bufferSize;

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(colliderDVB.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "デバックコライダーバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DebugRenderer::MapDVB()
{
	void* ptr = nullptr;

	hr = colliderDVB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "コライダーの各座標が収納されたDVBマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, debugColMat.data(), sizeof(DebugColMat) * debugColMat.size());

	colliderDVB->Unmap(0, nullptr);

	return true;
}

bool DebugRenderer::InitDBV()
{
	colliderDVBV.BufferLocation = colliderDVB->GetGPUVirtualAddress();
	colliderDVBV.SizeInBytes = sizeof(DebugColMat) * debugColMat.size();
	colliderDVBV.StrideInBytes = sizeof(DebugColMat);

	return true;
}

void DebugRenderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(1, 1, &colliderDVBV);
}

void DebugRenderer::Update()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	if (colList.size() != instanceCount)
	{
		RenderDebugCollider();

		bool isSuccess = true;

		isSuccess &= InitDebugVertexBuffer();

		isSuccess &= MapDVB();

		isSuccess &= InitDBV();

		if (!isSuccess)
			std::cerr << "DebugRendererの初期化失敗" << std::endl;
	}



	for (int i = 0;i < instanceCount;i++)
	{
		debugColMat[i].world = DirectX::XMMatrixTranspose(colList[i]->GetTRSMatrix());
		if (colList[i]->isColliding)
			debugColMat[i].color = XMFLOAT4(1, 0, 0, 0.3);
		else
			debugColMat[i].color = XMFLOAT4(0, 1, 0, 0.3);

	}

	MapDVB();
}

void DebugRenderer::InitRenderer()
{
	primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	RenderDebugCollider();

	bool isSuccess = true;

	isSuccess &= InitDebugVertexBuffer();

	isSuccess &= MapDVB();

	isSuccess &= InitDBV();

	if (!isSuccess)
		std::cerr << "DebugRendererの初期化失敗" << std::endl;

	LoadVertexShader(L"DebugVertexShader.cso");
	LoadPixelShader(L"DebugPixelShader.cso");

	D3D12_INPUT_ELEMENT_DESC param = { "COLLIDER_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);

	param = { "COLLIDER_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);

	param = { "COLLIDER_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);

	param = { "COLLIDER_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);

	param = { "COLORS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);
}

void DebugRenderer::RenderDebugCollider()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	instanceCount = colList.size();

	debugColMat.resize(instanceCount);

	for (int i = 0;i < instanceCount;i++)
	{
		debugColMat[i].world = DirectX::XMMatrixTranspose(colList[i]->GetTRSMatrix());
		if(colList[i]->isColliding)
			debugColMat[i].color = XMFLOAT4(1,0,0, 0.3);
		else
			debugColMat[i].color = XMFLOAT4(0, 1, 0, 0.3);
	
	}
}

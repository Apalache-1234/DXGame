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
	UINT bufferSize = sizeof(Instance) * instances.size() + 1;

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
		IID_PPV_ARGS(IVB.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "インスタンスデバックバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DebugRenderer::MapDVB()
{
	void* ptr = nullptr;

	hr = IVB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "インスタンスの各座標が収納されたDVBマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, instances.data(), sizeof(Instance) * instances.size());

	IVB->Unmap(0, nullptr);

	return true;
}

bool DebugRenderer::InitDBV()
{
	IVBV.BufferLocation = IVB->GetGPUVirtualAddress();
	IVBV.SizeInBytes = sizeof(Instance) * instances.size();
	IVBV.StrideInBytes = sizeof(Instance);

	return true;
}

void DebugRenderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(1, 1, &IVBV);
}

bool DebugRenderer::InitIVB()
{
	bool isSuccess = true;

	isSuccess &= InitDebugVertexBuffer();

	isSuccess &= MapDVB();

	isSuccess &= InitDBV();

	if (!isSuccess)
		std::cerr << "IDBの初期化失敗" << std::endl;

	return isSuccess;
}

void DebugRenderer::InitDebugRenderer()
{
}

void DebugRenderer::InitRenderer()
{
	InitDebugRenderer();

	InitIVB();



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

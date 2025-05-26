#include "DescriptorHeapManager.h"
#include "DXRenderSystem.h"

DescriptorHeapManager::DescriptorHeapManager() : DescriptorHeapManager(100)
{
}

DescriptorHeapManager::DescriptorHeapManager(UINT num)
{
	assert(num != 0);

	capacity = num;

	bool isSucceed = true;

	isSucceed &= InitDescHeap();

	isSucceed &= InitList();
}

DescriptorHeapManager::~DescriptorHeapManager()
{
	isDestroyed = true;

	free.clear();
}

void DescriptorHeapManager::BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle , D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	assert(free.size() < capacity );

	auto handle = std::make_unique<PtrHandles>(cpuHandle, gpuHandle, this);

	free.push_back(std::move(handle));
}

std::unique_ptr<DescriptorHeapManager::PtrHandles> DescriptorHeapManager::GetHandle()
{
	assert(free.size() >= 0);

	auto handles = std::move(free.back());

	free.pop_back();

	return handles;
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::GetHeap()
{
	return heap;
}

bool DescriptorHeapManager::IsDestroyed()
{
	return isDestroyed;
}

bool DescriptorHeapManager::InitDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	desc.NodeMask = 0;

	desc.NumDescriptors = capacity;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	HRESULT hr = DXRenderSystem::GetInstance().GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "デスクリプタヒープ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DescriptorHeapManager::InitList()
{
	assert(capacity != 0);

	free.reserve(capacity);

	auto increment = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0;i < capacity;i++)
	{
		auto cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
		auto gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();

		cpuHandle.ptr += increment * i;
		gpuHandle.ptr += increment * i;

		auto handle = std::make_unique<PtrHandles>(cpuHandle, gpuHandle, this);

		free.push_back(std::move(handle));
	}

	return true;
}

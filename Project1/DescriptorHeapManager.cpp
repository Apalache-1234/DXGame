#include "DescriptorHeapManager.h"
#include "DXRenderSystem.h"

DescriptorHeapManager::DescriptorHeapManager() : DescriptorHeapManager(100)
{
}

DescriptorHeapManager::DescriptorHeapManager(UINT num) : DescriptorHeapManager(num , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV , D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
{
}

DescriptorHeapManager::DescriptorHeapManager(UINT num , D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag)
{
	assert(num != 0);

	capacity = num;

	bool isSucceed = true;

	isSucceed &= InitDescHeap(type, flag);

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

	auto handles = std::move(free.front());

	free.erase(free.begin());

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

bool DescriptorHeapManager::InitDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE type , D3D12_DESCRIPTOR_HEAP_FLAGS flag)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = flag;

	desc.NodeMask = 0;

	desc.NumDescriptors = capacity;

	desc.Type = type;

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

	auto increment = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(heap->GetDesc().Type);

	for (int i = 0;i < capacity;i++)
	{
		auto cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += increment * i;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

		if (heap->GetDesc().Flags != D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
		{
			gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
			gpuHandle.ptr += increment * i;
		}

		auto handle = std::make_unique<PtrHandles>(cpuHandle, gpuHandle, this);

		free.push_back(std::move(handle));
	}

	return true;
}

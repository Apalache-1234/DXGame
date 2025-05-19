#include "DescriptorHeapManager.h"

DescriptorHeapManager::DescriptorHeapManager() : DescriptorHeapManager(100)
{
}

DescriptorHeapManager::DescriptorHeapManager(UINT num)
{
	assert(num != 0);

	capacity = num;

	bool isSucceed;

	isSucceed &= InitDescHeap();

	isSucceed &= InitList();
}

DescriptorHeapManager::~DescriptorHeapManager()
{
}

DescriptorHeapManager::PtrHandles DescriptorHeapManager::GetHandle()
{
	assert(free.size() >= 0);

	PtrHandles handles = std::move(free.back());

	free.pop_back();

	return std::move(handles);
}

void DescriptorHeapManager::BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle , D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{

}

ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::GetHeap()
{
	return heap;
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

		free.emplace_back(cpuHandle, gpuHandle, this);
	}

	return true;
}

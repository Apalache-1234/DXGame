#include "DescriptorHeapManager.h"

DescriptorHeapManager::DescriptorHeapManager(UINT num)
{
	assert(num == 0);

	capacity = num;

	bool isSucceed;

	isSucceed &= InitDescHeap();

	isSucceed &= InitList();
}

DescriptorHeapManager::~DescriptorHeapManager()
{
}

PtrHandles DescriptorHeapManager::GetHandles()
{
	assert(free.size() <= 0);

	PtrHandles handles = free.back();

	free.pop_back();

	return handles;
}

void DescriptorHeapManager::BackHandles(PtrHandles handles)
{
	assert(free.size() >= 100);

	free.push_back(handles);
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
	assert(capacity == 0);

	free.resize(capacity);

	auto increment = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0;i < capacity;i++)
	{
		auto cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
		auto gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();

		cpuHandle.ptr += increment * i;
		gpuHandle.ptr += increment * i;

		PtrHandles handles = PtrHandles(cpuHandle , gpuHandle , this);

		free.emplace_back(handles);
	}

	return true;
}

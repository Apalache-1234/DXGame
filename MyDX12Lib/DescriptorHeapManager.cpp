#include "PCH/pch.h"
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

void DescriptorHeapManager::BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle , D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle , int idx)
{
	HandleStatus status = {};

	free[idx]->handle = std::make_unique<PtrHandles>(cpuHandle, gpuHandle, this);
	free[idx]->handle->idx = idx;

	free[idx]->isEmpty = false;
}

std::unique_ptr<PtrHandles> DescriptorHeapManager::GetHandle()
{
	int temp = nextIndex;

	while (free[nextIndex]->isEmpty)
	{
		nextIndex = (nextIndex + 1) % free.size();

		if (temp == nextIndex)
			assert(true);
	}

	auto handles = std::move(free[nextIndex]->handle);
	free[nextIndex]->isEmpty = true;
	nextIndex = (nextIndex + 1) % free.size();

	return handles;
}

std::vector<std::unique_ptr<PtrHandles>> DescriptorHeapManager::GetSerialHandle(UINT num)
{
	int temp = nextIndex;

	bool isFound = false;

	std::vector<std::unique_ptr<PtrHandles>> list;

	if (num == 1)
	{
		list.push_back(GetHandle());
		return list;
	}

	while (!isFound)
	{
		while (free[nextIndex]->isEmpty)
		{
			nextIndex = (nextIndex + 1) % free.size();

			if (temp == nextIndex)
				assert(true);
		}

		for (UINT i = 1;i < num;i++)
		{
			if (nextIndex + i == free.size())
			{
				nextIndex = 0;
				break;
			}

			if (free[nextIndex + i]->isEmpty)
			{
				nextIndex = (nextIndex + i + 1) % free.size();
				break;
			}
			else if (i == num - 1)
			{
				isFound = true;
			}
		}
	}

	for (UINT i = 0;i < num;i++)
	{
		list.push_back(std::move(free[nextIndex + i]->handle));

		free[nextIndex + i]->isEmpty = true;
	}

	nextIndex = (nextIndex + num ) % free.size();

	return list;
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

	for (UINT i = 0;i < capacity;i++)
	{
		free.push_back(std::make_unique<HandleStatus>());

		auto cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += increment * i;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

		if (heap->GetDesc().Flags != D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
		{
			gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
			gpuHandle.ptr += increment * i;
		}

		free.back()->handle = std::make_unique<PtrHandles>(cpuHandle, gpuHandle, this);
		free.back()->handle->idx = i;
	}

	return true;
}

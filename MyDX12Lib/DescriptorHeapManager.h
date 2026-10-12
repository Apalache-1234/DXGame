#ifndef DESCRIPTORHEAPMANAGER_H
#define DESCRIPTORHEAPMANAGER_H

#include <iostream>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

struct HandleStatus;

class PtrHandles;

class DescriptorHeapManager
{
public:
	DescriptorHeapManager();
	DescriptorHeapManager(UINT num);

	DescriptorHeapManager(UINT num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag);

	DescriptorHeapManager(const DescriptorHeapManager&) = delete;
	DescriptorHeapManager& operator=(const DescriptorHeapManager&) = delete;

	~DescriptorHeapManager();

	void BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle , int idx);

	std::unique_ptr<PtrHandles> GetHandle();

	std::vector<std::unique_ptr<PtrHandles>> GetSerialHandle(UINT num);

	ComPtr<ID3D12DescriptorHeap> GetHeap();

	bool IsDestroyed();
private:
	bool InitDescHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag);

	/// <summary>
    /// デスクリプタヒープ
    /// </summary>
	ComPtr<ID3D12DescriptorHeap> heap;

	std::vector<std::unique_ptr<HandleStatus>> free;

	bool InitList();

	UINT capacity = 0;

	bool isDestroyed = false;

	UINT nextIndex = 0;
};


struct PtrHandles
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

	int idx = 0;

	PtrHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuPtr, D3D12_GPU_DESCRIPTOR_HANDLE gpuPtr, DescriptorHeapManager* heap) : cpuHandle(cpuPtr), gpuHandle(gpuPtr), descHeap(heap) {}

	void ReturnHandle()
	{
		if (!descHeap->IsDestroyed())
			descHeap->BackHandles(cpuHandle, gpuHandle, idx);
	}

	~PtrHandles()
	{

	}
private:
	DescriptorHeapManager* descHeap = nullptr;
};

namespace std
{
	template<>
	struct default_delete<PtrHandles>
	{
		void operator()(PtrHandles* handle) const
		{
			handle->ReturnHandle();

			delete handle;
		}
	};
};


struct HandleStatus
{
	std::unique_ptr<PtrHandles> handle;

	bool isEmpty = false;
};

#endif
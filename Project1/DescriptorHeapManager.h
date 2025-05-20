#include <iostream>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "DXRenderSystem.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

#ifndef DESCRIPTORHEAPMANAGER_H
#define DESCRIPTORHEAPMANAGER_H

class DescriptorHeapManager
{
public:
	DescriptorHeapManager();
	DescriptorHeapManager(UINT num);

	DescriptorHeapManager(const DescriptorHeapManager&) = delete;
	DescriptorHeapManager& operator=(const DescriptorHeapManager&) = delete;

	~DescriptorHeapManager();

	void BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	struct PtrHandles
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

		PtrHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuPtr, D3D12_GPU_DESCRIPTOR_HANDLE gpuPtr, DescriptorHeapManager* heap) : cpuHandle(cpuPtr), gpuHandle(gpuPtr), descHeap(heap) {}

		void ReturnHandle()
		{
			descHeap->BackHandles(cpuHandle , gpuHandle);
		}

		~PtrHandles()
		{

		}
	private:
		DescriptorHeapManager* descHeap = nullptr;
	};

	std::unique_ptr<DescriptorHeapManager::PtrHandles> GetHandle();

	ComPtr<ID3D12DescriptorHeap> GetHeap();
private:
	/// <summary>
    /// デスクリプタヒープ
    /// </summary>
	ComPtr<ID3D12DescriptorHeap> heap;

	std::vector<std::unique_ptr<PtrHandles>> free;

	bool InitDescHeap();

	bool InitList();

	UINT capacity = 0;
};

namespace std
{
	template<>
	struct default_delete<DescriptorHeapManager::PtrHandles>
	{
		void operator()(DescriptorHeapManager::PtrHandles* handle) const
		{
			handle->ReturnHandle();

			delete handle;
		}
	};
};

#endif
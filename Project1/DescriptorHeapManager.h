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

	struct PtrHandles
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

		PtrHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuPtr, D3D12_GPU_DESCRIPTOR_HANDLE gpuPtr, DescriptorHeapManager* heap) : cpuHandle(cpuPtr), gpuHandle(gpuPtr)
		{
			descHeap = heap;
		}

		PtrHandles(const PtrHandles&) = delete;
		PtrHandles& operator=(const PtrHandles&) = delete;

		PtrHandles(PtrHandles&&) noexcept = default;
		PtrHandles& operator=(PtrHandles&&) noexcept = default;

		PtrHandles() = default;

		~PtrHandles()
		{
			if (!isDeleted) {
		
				descHeap->BackHandles(this->cpuHandle , this->gpuHandle);
			}
		}

		DescriptorHeapManager* descHeap = nullptr;

		bool isDeleted = false;
	private:
	};

	PtrHandles GetHandle();

	void BackHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	ComPtr<ID3D12DescriptorHeap> GetHeap();
private:
	/// <summary>
    /// デスクリプタヒープ
    /// </summary>
	ComPtr<ID3D12DescriptorHeap> heap;

	std::vector<PtrHandles> free;

	bool InitDescHeap();

	bool InitList();

	UINT capacity = 0;
};

#endif
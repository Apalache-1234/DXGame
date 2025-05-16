#ifndef DESCRIPTORHEAPMANAGER_H
#define DESCRIPTORHEAPMANAGER_H

#include "Component.h"
#include <iostream>
#include <d3d12.h>
#include <d3dx12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_4.h>
#pragma comment(lib, "dxgi.lib")
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
#pragma comment(lib, "dxguid.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <ResourceUploadBatch.h>
#include <DDSTextureLoader.h>
#include <VertexTypes.h>
#include "Camera.h"
#include "Window.h"
#undef min
#undef max
#include "DXRenderSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "TextureLoader.h"
#include "MeshLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

struct PtrHandles;

class DescriptorHeapManager
{
public:
	DescriptorHeapManager(UINT num);

	~DescriptorHeapManager();

	PtrHandles GetHandles();

	void BackHandles(PtrHandles handles);
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

struct PtrHandles
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

	PtrHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpuPtr, D3D12_GPU_DESCRIPTOR_HANDLE gpuPtr, DescriptorHeapManager* heap) : cpuHandle(cpuPtr), gpuHandle(gpuPtr)
	{
		descHeap = std::shared_ptr<DescriptorHeapManager>(heap);
	}

	PtrHandles() = default;

	~PtrHandles()
	{
		auto handles = this;

		descHeap->BackHandles(*handles);
	}
private:
	std::shared_ptr<DescriptorHeapManager> descHeap;
};

#endif
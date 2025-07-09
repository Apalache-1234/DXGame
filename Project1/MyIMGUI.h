#ifndef MYIMGUI_H
#define MYIMGUI_H

#include <memory>
#include <iostream>
#include <unordered_map>
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "Window.h"
#include "imgui_impl_win32.h"
#include "DXRenderSystem.h"
#include "Flags.h"
#include "SystemManager.h"
#include "Collider.h"
#include "Rigidbody.h"
#include <iomanip>
#include <sstream>

class MyIMGUI
{
public:
	MyIMGUI();

	virtual ~MyIMGUI();

	static MyIMGUI& GetInstance();

	void Init();

	void Term();

	void MainLoop();

	ComPtr<ID3D12DescriptorHeap> GetHeap();

	// Simple free list based allocator
	struct ExampleDescriptorHeapAllocator
	{
		ID3D12DescriptorHeap* Heap = nullptr;
		D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
		UINT                        HeapHandleIncrement;
		ImVector<int>               FreeIndices;

		void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
		{
			IM_ASSERT(Heap == nullptr && FreeIndices.empty());
			Heap = heap;
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			HeapType = desc.Type;
			HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
			HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
			HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
			FreeIndices.reserve((int)desc.NumDescriptors);
			for (int n = desc.NumDescriptors; n > 0; n--)
				FreeIndices.push_back(n - 1);
		}
		void Destroy()
		{
			Heap = nullptr;
			FreeIndices.clear();
		}
		void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			IM_ASSERT(FreeIndices.Size > 0);
			int idx = FreeIndices.back();
			FreeIndices.pop_back();
			out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
			out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
		}
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
		{
			int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
			int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
			IM_ASSERT(cpu_idx == gpu_idx);
			FreeIndices.push_back(cpu_idx);
		}
	};

	/// <summary>
	/// IMGUIを使用するかどうか
	/// </summary>
	const bool useIMGUI = true;
private:
	/// <summary>
    /// IMGUIをカスタムするためのクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<MyIMGUI> instance;

	/// <summary>
	/// IMGUI用のSRVのヒープ
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> forIMGUIHeap;
};

#endif
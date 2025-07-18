#ifndef DXRENDERSYSTEM_H
#define DXRENDERSYSTEM_H

#include <windows.h>
#include <memory>
#include <tchar.h>
#include "Window.h"
#include <iostream>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <ResourceUploadBatch.h>
#include <DDSTextureLoader.h>
#include <VertexTypes.h>
#include "wrl.h"
#include "DescriptorHeapManager.h"
#include <optional>
#include "SystemManager.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer;

class DXRenderSystem
{
public:
	DXRenderSystem();

	~DXRenderSystem();

	static DXRenderSystem& GetInstance();

	bool Init();

	const static UINT msaaCount = 8;
	UINT msaaQuality = 0;

	ComPtr<ID3D12CommandQueue>& GetCommandQueue();

	ComPtr<ID3D12GraphicsCommandList>& GetCommandList();

	bool CreateMSAAView();

	void WaitForRender();

	template <typename T>
	void RenderComponent()
	{
		auto rendL = SystemManager::GetInstance().GetComponentList<T>();

		for (int i = 0; i < rendL.size(); i++)
		{
			cmd_list->RSSetScissorRects(1, &scissor);
			cmd_list->RSSetViewports(1, &viewport);

			auto rend = dynamic_cast<Renderer*>(rendL[i].get());

			rend->AddRenderCommand(cmd_list, backbuffer_index);
		}
	}

	void RenderPostEffect();

	void RenderIMGUI();

	void Draw();

	void ExecuteCommands();

	ComPtr<ID3D12Device>& GetDevice();

	ComPtr<ID3D12Resource>& GetEmptyUploadHeap();

	void SetDeletingHeap(ComPtr<ID3D12Resource> heap);

	void SetDeletingComps(shared_ptr<Component> comp);

	void SetDeletingComps(std::vector<shared_ptr<Component>> comps);

	const static int frameBuffer = 2;
private:
	ComPtr<ID3D12Resource> postEffect;

	std::vector<ComPtr<ID3D12Resource>> uploadHeaps;

	std::vector<ComPtr<ID3D12Resource>> deletingHeaps;

	std::vector<shared_ptr<Component>> deletingComps;

	shared_ptr<Entity> postEffectEntity;

	/// <summary>
	/// DirectX12のデバイスを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateDevice();

	/// <summary>
	/// コマンドキューを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateCommandQueue();

	/// <summary>
	/// モニタなどのハードウェアとのデータの受け渡しを行うファクトリーを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateFactory();

	/// <summary>
	/// スワップチェインを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateSwapChain();

	/// <summary>
	/// コマンドアロケータを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateCommandAllocater();

	/// <summary>
	/// コマンドリストを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateCommandList();

	bool CheckMSAASupport();

	/// <summary>
	/// レンダーターゲットビュー用のデスクリプタヒープを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateRTVDescriptorHeap();

	/// <summary>
	/// レンダーターゲットビューを作成しハンドルにCPU上の場所を格納する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateRTV();

	bool CreateMSAAResource();

	/// <summary>
	/// 描画コマンドを実行して次に行くまでの待機に必要なフェンスを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateFence();

	bool CreatePostEffectResouce();

	bool CreatePostEffectView();

	/// <summary>
	/// 深度バッファビューのリソースを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateDBVResouce();

	/// <summary>
	/// 深度バッファビュー用のデスクリプタヒープを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateDBVDescripterHeap();

	/// <summary>
	/// 深度バッファビューを作成しハンドルにCPU上の場所を格納する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateDBV();

	/// <summary>
	/// `描画の領域を決めるシザーとビューポートを設定する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool SetScissorAndViewPort();

	/// <summary>
	/// DirectXの基幹システムを構築するクラスのシングルトンのインスタンス
	/// </summary>
	static std::unique_ptr<DXRenderSystem> instance;

	/// <summary>
	/// WinWPI系の関数の可否を格納する変数
	/// </summary>
	HRESULT hr;

	float colorBG[4] =
	{ 0.5 , 0.5 , 0.5, 1 };


	ComPtr<ID3D12Device> device;
	ComPtr<IDXGIAdapter1> adapter1;
	ComPtr<IDXGIFactory1> factory;
	ComPtr<IDXGISwapChain3> swap_chain;
	ComPtr<ID3D12CommandQueue> cmd_queue;
	ComPtr<ID3D12CommandAllocator> cmd_allocator[frameBuffer];
	ComPtr<ID3D12Resource> depth_buffer;
	ComPtr<ID3D12GraphicsCommandList> cmd_list;
	std::optional<DescriptorHeapManager> heapRT;
	std::optional<DescriptorHeapManager> heapDS;
	std::optional<DescriptorHeapManager> heap;
	ComPtr<ID3D12Resource> rtv;
	ComPtr<ID3D12Resource> color_buffer[frameBuffer];
	ComPtr<ID3D12Fence> fence;
	UINT16 fence_counter = 1;

	D3D12_CLEAR_VALUE clearDepth = {};

	ComPtr<ID3D12Resource> msaaRT;
	ComPtr<ID3D12Resource> msaaDS;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;

	HANDLE fence_event;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> RTVHandle[frameBuffer];
	std::unique_ptr<DescriptorHeapManager::PtrHandles> msaaRTVHandle;

	std::unique_ptr < DescriptorHeapManager::PtrHandles> DSVHandle;
	std::unique_ptr < DescriptorHeapManager::PtrHandles> msaaDSVHandle;

	std::unique_ptr < DescriptorHeapManager::PtrHandles> postEffectHandle;

	int backbuffer_index = 0;
};

#endif

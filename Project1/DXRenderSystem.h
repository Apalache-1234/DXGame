#include <windows.h>
#include <memory>
#include <tchar.h>
#include "Window.h"
#include "Renderer.h"

#include <iostream>
#include <DirectXMath.h>
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
#include "MyIMGUI.h"
#include "SkyBox.h"
#include "UIRenderer.h"
#include "wrl.h"
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;


#ifndef DXRENDERSYSTEM_H
#define DXRENDERSYSTEM_H

class Renderer;

class DXRenderSystem
{
public:
	DXRenderSystem();

	~DXRenderSystem();

	static DXRenderSystem& GetInstance();

	bool Init();

	ComPtr<ID3D12CommandQueue>& GetCommandQueue();

	ComPtr<ID3D12GraphicsCommandList>& GetCommandList();

	void WaitForRender();

	void Draw();

	ComPtr<ID3D12Device>& GetDevice();

	const static int frameBuffer = 2;
private:

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

	/// <summary>
	/// 描画コマンドを実行して次に行くまでの待機に必要なフェンスを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateFence();

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
	ComPtr<ID3D12DescriptorHeap> heap_rtv;
	ComPtr<ID3D12DescriptorHeap> heap_dbv;
	ComPtr<ID3D12Resource> rtv;
	ComPtr<ID3D12Resource> color_buffer[frameBuffer];
	ComPtr<ID3D12Fence> fence;
	UINT16 fence_counter = 1;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;

	HANDLE fence_event;

	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle[frameBuffer] = {};

	D3D12_CPU_DESCRIPTOR_HANDLE dbv_handle = {};

	int backbuffer_index = 0;
};

#endif

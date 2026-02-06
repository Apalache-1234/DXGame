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
#include "Camera.h"
#include <optional>
#include "SystemManager.h"
#include "Utility.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer;

class DirectionalLight;
class SpotLight;

class DXRenderSystem
{
public:
	DXRenderSystem();

	~DXRenderSystem();

	UINT GetCurrentShadowMapIndex();

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
			if (isShadowMapRendering)
			{
				cmd_list->RSSetScissorRects(1, &shadowScissor);
				cmd_list->RSSetViewports(1, &shadowViewport);
			}
			else
			{
				cmd_list->RSSetScissorRects(1, &scissor);
				cmd_list->RSSetViewports(1, &viewport);
			}

			auto rend = dynamic_cast<Renderer*>(rendL[i].get());

			rend->AddRenderCommand(cmd_list, backbuffer_index);
		}
	}

	void RenderPostEffect(bool whenOnUi);

	void RenderIMGUI();

	void  ComputeDLShadowMapRender(std::shared_ptr<DirectionalLight> dl);

	void ComputeSLShadowMapRender(std::shared_ptr<SpotLight> sl);

	void RenderShadowMap();

	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapResourceCPUHandle(UINT index);
	ComPtr<ID3D12Resource> GetShadowMapResource();

	ComPtr<ID3D12RootSignature> GetRootSignature();

	void AddRootParam(D3D12_ROOT_PARAMETER param);

	int GetRootParamNum();

	void Map();

	void Draw();

	std::shared_ptr<DescriptorHeapManager> GetHeapManager();

	void ExecuteCommands();

	ComPtr<ID3D12Device>& GetDevice();

	ComPtr<ID3D12Resource>& GetEmptyUploadHeap();

	void SetDeletingHeap(ComPtr<ID3D12Resource> heap);

	void SetDeletingComps(shared_ptr<Component> comp);

	void SetDeletingComps(std::vector<shared_ptr<Component>> comps);

	void SetDeletingEnt(shared_ptr<Entity> ent);
	void SetDeletingResource(ComPtr<ID3D12Resource> rsc);

	bool GetIsShadowMapRendering();

	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapTextureSRVCPUHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapTextureSRVGPUHandle();

	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowMapTextureSliceSRVCPUHandle(UINT ind);

	D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapTextureSliceSRVGPUHandle(UINT ind);

	void UpdateLightCastingShadow();

	UINT GetShadowMapNum();

	void SetCascadeShadowCount(UINT num);

	void SetCascadeShadowFarZ(float val);
	void SetCascadeShadowNearZ(float val);

	void SetCascadeShadowLerp(float val);

	UINT GetCascadeShadowCount();

	float GetCascadeShadowFarZ();
	float GetCascadeShadowNearZ();

	float GetCascadeShadowLerp();

	DirectX::XMFLOAT3 GetCascadeShadowDist();

	void SetFogColor(DirectX::XMVECTOR color);

	void SetFogRange(float range);

	DirectX::XMVECTOR GetFogColor();

	float GetFogRange();

	bool fogFlag = false;

	DirectX::XMFLOAT3 GetAmbientColor();

	void SetAmbientColor(DirectX::XMFLOAT3 col);

	const static int frameBuffer = 2;
private:
	struct DirLight
	{
		DirectX::XMMATRIX matVP[4];

		DirectX::XMFLOAT3 direct;

		DirectX::XMFLOAT3 color;

		float power;

		float shadow;

		float shadowMapIndex = 0;

		DirectX::XMFLOAT3 padding;
	};

	struct PLight
	{
		DirectX::XMMATRIX matVP;

		float radius;

		DirectX::XMFLOAT3 color;

		DirectX::XMFLOAT3 pos;

		float power;

		float shadow;

		float shadowMapIndex = 0;

		DirectX::XMFLOAT2 padding;
	};

	struct SLight
	{
		DirectX::XMMATRIX matVP;

		float angular;

		DirectX::XMFLOAT3 direct;

		DirectX::XMFLOAT3 color;

		DirectX::XMFLOAT3 pos;

		float power;

		float length;

		float shadow;

		float shadowMapIndex = 0;

		DirectX::XMFLOAT2 padding;
	};

	int shadowMapRes = 2048;

	UINT cascadeShadowCount = 4;

	float CSDividedDist[4];

	float CSFarZ = 100;
	float CSNearZ = 0.1f;

	float cascadeShadowLerp = 0.9f;

	std::vector<ComPtr<ID3D12Resource>> uploadHeaps;

	std::shared_ptr<DescriptorHeapManager> heapManager;

	std::vector<ComPtr<ID3D12Resource>> deletingHeaps;

	std::vector<shared_ptr<Component>> deletingComps;
	std::vector<std::pair<int , shared_ptr<Entity>>> deletingEnts;
	std::vector<std::pair<int , ComPtr<ID3D12Resource>>> deletingResources;

	shared_ptr<Entity> postEffectEntity;

	bool isShadowMapRendering = false;

	DirectX::XMVECTOR fogColor = DirectX::XMVectorSplatOne();

	float fogRange = 1000;

	std::vector<DirLight> DirLights;

	ComPtr<ID3D12Resource> DLBuffer[frameBuffer];

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> DLBHandle;

	std::vector<PLight> PLights;

	ComPtr<ID3D12Resource> PLBuffer[frameBuffer];

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> PLBHandle;

	std::vector<SLight> SLights;

	ComPtr<ID3D12Resource> SLBuffer[frameBuffer];

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> SLBHandle;

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> lightInfoCBHandle;

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> timeInfoCBHandle;

	ComPtr<ID3D12Resource> lightInfoCB[frameBuffer];

	ComPtr<ID3D12Resource> timeInfoCB[frameBuffer];

	D3D12_CONSTANT_BUFFER_VIEW_DESC lightInfoCBView[frameBuffer];

	D3D12_CONSTANT_BUFFER_VIEW_DESC timeInfoCBView[frameBuffer];

	LightInfo lightInfo;

	TimeInfo timeInfo;

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

	void CreateCSDividedDist();

	void ResetCommands();

	void RenderDLShadowMap(std::shared_ptr<DirectionalLight> dl, int ind, XMMATRIX proj, XMMATRIX view);

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

	bool CreateShadowMapResource();

	bool CreateShadowMapView();

	void CreateShadowMapTextureSRV();

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

	bool UpdateLightCastingShadowNum();

	bool InitDirectionalLightBResource();

	bool MapDLB();

	bool InitDLBV();

	bool InitPointLightBResource();

	bool MapPLB();

	bool InitPLBV();

	bool InitSpotLightBResource();

	bool MapSLB();

	bool InitSLBV();

	bool InitLightInfoCB();

	bool LinkLightInfoCBVToDescHeap();

	bool MapLLICB();

	bool InitTimeInfoCB();

	bool LinkTimeInfoCBVToDescHeap();

	bool MapTICB();

	void PrepareForRootSignature();

	/// <summary>
    /// ルートシグネチャを作成する
    /// </summary>
    ///  <returns>正常に終了したらtrueを返す</returns>
	bool CreateRootSignature();

	/// <summary>
	/// DirectXの基幹システムを構築するクラスのシングルトンのインスタンス
	/// </summary>
	static std::unique_ptr<DXRenderSystem> instance;

	/// <summary>
	/// WinWPI系の関数の可否を格納する変数
	/// </summary>
	HRESULT hr;

	float colorBG[4] =
	{ 0 , 0 , 0, 1 };

	PtrHandle shadowMapTexHandle;
	std::vector<PtrHandle> shadowMapTexSliceHandles;

	std::vector<D3D12_ROOT_PARAMETER> param;

	std::vector<D3D12_STATIC_SAMPLER_DESC> descSamplers;

	D3D12_DESCRIPTOR_RANGE rangeCB;
	D3D12_DESCRIPTOR_RANGE rangeSRV[3];

	D3D12_DESCRIPTOR_RANGE shadowMapSRV[1];

	/// <summary>
    /// ルートシグネチャのCOM
    /// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

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
	std::optional<DescriptorHeapManager> heapSM;
	ComPtr<ID3D12Resource> postEffect[2];
	ComPtr<ID3D12Resource> shadowMap;
	ComPtr<ID3D12Resource> rtv;
	ComPtr<ID3D12Resource> color_buffer[frameBuffer];
	ComPtr<ID3D12Fence> fence;
	UINT64 fence_counter[frameBuffer] = {0 , 0};

	D3D12_CLEAR_VALUE clearDepth = {};

	ComPtr<ID3D12Resource> msaaRT;
	ComPtr<ID3D12Resource> msaaDS;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;

	UINT DlightCastingShadowNum = 0;
	UINT SlightCastingShadowNum = 0;
	UINT PlightCastingShadowNum = 0;

	D3D12_VIEWPORT shadowViewport;
	D3D12_RECT shadowScissor;

	HANDLE fence_event;

	XMMATRIX ambientVPMat;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> RTVHandle[frameBuffer];
	std::unique_ptr<DescriptorHeapManager::PtrHandles> msaaRTVHandle;

	std::unique_ptr < DescriptorHeapManager::PtrHandles> DSVHandle;
	std::unique_ptr < DescriptorHeapManager::PtrHandles> msaaDSVHandle;

	std::unique_ptr < DescriptorHeapManager::PtrHandles> postEffectHandle[2];

	std::vector<std::unique_ptr < DescriptorHeapManager::PtrHandles>> shadowMapHandle;

	int backbuffer_index = 0;
	int frontbuffer_index = 1;

	UINT currentShadowMapIndex = 0;

	DirectX::XMFLOAT3 ambientColor = DirectX::XMFLOAT3(1, 1, 1);
};

#endif
